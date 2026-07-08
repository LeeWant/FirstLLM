#include "firstllm/model/gguf_reader.h"

#include <array>
#include <fstream>
#include <istream>
#include <limits>
#include <utility>
#include <variant>

namespace firstllm {
namespace {

// GGUF 文件魔数，按 little-endian 读取后等于字符序列 "GGUF"。
constexpr std::uint32_t kGgufMagic = 0x46554747U;

// GGUF/GGML 当前常见 tensor 维度上限。超过这个值通常表示文件损坏或格式不匹配。
constexpr std::uint32_t kMaxTensorDimensions = 4;

// GGUF 未显式声明 general.alignment 时，数据区默认按 32 字节对齐。
constexpr std::uint64_t kDefaultDataAlignment = 32;

// 从输入流读取固定字节数；返回 false 表示文件截断或读取失败。
bool ReadBytes(std::istream* input, std::uint8_t* data, std::size_t size) {
  input->read(reinterpret_cast<char*>(data),
              static_cast<std::streamsize>(size));
  return input->gcount() == static_cast<std::streamsize>(size);
}

// 读取 1 字节无符号整数，当前主要用于 bool metadata。
Status ReadUint8(std::istream* input, std::uint8_t* value) {
  if (!ReadBytes(input, value, sizeof(*value))) {
    return Status::InvalidArgument("GGUF file is truncated");
  }

  return Status::Ok();
}

// 按 little-endian 读取 uint32，避免依赖本机字节序。
Status ReadUint32(std::istream* input, std::uint32_t* value) {
  // bytes 保存从文件中读出的原始 4 字节。
  std::array<std::uint8_t, 4> bytes{};

  if (!ReadBytes(input, bytes.data(), bytes.size())) {
    return Status::InvalidArgument("GGUF file is truncated");
  }

  *value = static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);

  return Status::Ok();
}

// 按 little-endian 读取 uint64，GGUF 的计数字段和字符串长度会用到它。
Status ReadUint64(std::istream* input, std::uint64_t* value) {
  // bytes 保存从文件中读出的原始 8 字节。
  std::array<std::uint8_t, 8> bytes{};

  if (!ReadBytes(input, bytes.data(), bytes.size())) {
    return Status::InvalidArgument("GGUF file is truncated");
  }

  *value = 0;

  for (std::size_t i = 0; i < bytes.size(); ++i) {
    *value |= static_cast<std::uint64_t>(bytes[i]) << (8 * i);
  }

  return Status::Ok();
}

// 读取 GGUF string：uint64 长度 + 不带结尾 '\0' 的字节内容。
Status ReadString(std::istream* input, std::string* value) {
  // length 是字符串字节长度，不包含额外终止符。
  std::uint64_t length = 0;
  Status status = ReadUint64(input, &length);
  if (!status.ok()) {
    return status;
  }

  if (length > static_cast<std::uint64_t>(
                   std::numeric_limits<std::streamsize>::max())) {
    return Status::InvalidArgument("GGUF string is too large");
  }

  // text 是最终字符串缓冲区，长度先按 GGUF 记录分配好。
  std::string text(static_cast<std::size_t>(length), '\0');

  if (!text.empty()) {
    input->read(text.data(), static_cast<std::streamsize>(text.size()));
    if (input->gcount() != static_cast<std::streamsize>(text.size())) {
      return Status::InvalidArgument("GGUF file is truncated");
    }
  }

  *value = std::move(text);
  return Status::Ok();
}

// 读取当前输入流的文件绝对位置。这里用于定位 tensor info 之后的数据区起点。
Status CurrentStreamOffset(std::istream* input, std::uint64_t* offset) {
  const std::istream::pos_type position = input->tellg();
  if (position == std::istream::pos_type(-1)) {
    return Status::InvalidArgument("Failed to read GGUF stream position");
  }

  const std::streamoff raw_offset =
      static_cast<std::streamoff>(position);
  if (raw_offset < 0) {
    return Status::InvalidArgument("GGUF stream position is invalid");
  }

  *offset = static_cast<std::uint64_t>(raw_offset);
  return Status::Ok();
}

// 从 metadata 中查找 data section 对齐值。GGUF 没写 general.alignment 时使用默认 32。
Status GetDataAlignment(const std::vector<GgufMetadataEntry>& metadata,
                        std::uint64_t* alignment) {
  *alignment = kDefaultDataAlignment;

  // entry 是当前 metadata 条目；这里只关心 general.alignment。
  for (const GgufMetadataEntry& entry : metadata) {
    if (entry.key != "general.alignment") {
      continue;
    }

    if (const auto* value = std::get_if<std::uint32_t>(&entry.value)) {
      *alignment = static_cast<std::uint64_t>(*value);
    } else if (const auto* value =
                   std::get_if<std::uint64_t>(&entry.value)) {
      *alignment = *value;
    } else {
      return Status::InvalidArgument(
          "GGUF general.alignment metadata type is invalid");
    }
  }

  if (*alignment == 0) {
    return Status::InvalidArgument("GGUF data alignment is invalid");
  }

  return Status::Ok();
}

// 将 offset 向上对齐到 alignment 的整数倍，避免直接读取到 padding 区。
Status AlignOffset(std::uint64_t offset,
                   std::uint64_t alignment,
                   std::uint64_t* aligned_offset) {
  if (alignment == 0) {
    return Status::InvalidArgument("GGUF data alignment is invalid");
  }

  const std::uint64_t remainder = offset % alignment;
  if (remainder == 0) {
    *aligned_offset = offset;
    return Status::Ok();
  }

  const std::uint64_t padding = alignment - remainder;
  if (offset > std::numeric_limits<std::uint64_t>::max() - padding) {
    return Status::InvalidArgument("GGUF data section offset overflows");
  }

  *aligned_offset = offset + padding;
  return Status::Ok();
}

// 根据 data section 起点，把每个 tensor 的相对 offset 转换成文件绝对 offset。
Status FillTensorDataOffsets(std::uint64_t data_section_offset,
                             std::vector<GgufTensorInfo>* tensor_infos) {
  // info 是当前 tensor 的结构信息；offset 来自 GGUF，data_offset 是计算结果。
  for (GgufTensorInfo& info : *tensor_infos) {
    if (info.offset >
        std::numeric_limits<std::uint64_t>::max() - data_section_offset) {
      return Status::InvalidArgument("GGUF tensor data offset overflows");
    }

    info.data_offset = data_section_offset + info.offset;
  }

  return Status::Ok();
}

// 读取并校验 GGUF header。这个 helper 被 read_header/read_metadata 复用。
Status ReadHeader(std::istream* input, GgufHeader* header) {
  Status status = ReadUint32(input, &header->magic);
  if (!status.ok()) {
    return status;
  }

  status = ReadUint32(input, &header->version);
  if (!status.ok()) {
    return status;
  }

  status = ReadUint64(input, &header->tensor_count);
  if (!status.ok()) {
    return status;
  }

  status = ReadUint64(input, &header->metadata_kv_count);
  if (!status.ok()) {
    return status;
  }

  if (header->magic != kGgufMagic) {
    return Status::InvalidArgument("GGUF magic mismatch");
  }

  return Status::Ok();
}

// 打开输入文件，并统一处理空路径和文件不存在错误。
Status OpenInput(const std::string& path, std::ifstream* input) {
  if (path.empty()) {
    return Status::InvalidArgument("GGUF path is empty");
  }

  input->open(path, std::ios::binary);

  if (!*input) {
    return Status::NotFound("GGUF file not found: " + path);
  }

  return Status::Ok();
}

// 读取 metadata 列表。调用方负责先读取 header。
Status ReadMetadataEntries(std::istream* input,
                           std::uint64_t metadata_kv_count,
                           std::vector<GgufMetadataEntry>* metadata) {
  metadata->clear();

  // i 是 metadata 条目索引，数量来自 header.metadata_kv_count。
  for (std::uint64_t i = 0; i < metadata_kv_count; ++i) {
    // entry 保存当前正在读取的一条 key/value。
    GgufMetadataEntry entry{};

    Status status = ReadString(input, &entry.key);
    if (!status.ok()) {
      return status;
    }

    // raw_type 是文件中记录的原始类型编号。
    std::uint32_t raw_type = 0;
    status = ReadUint32(input, &raw_type);
    if (!status.ok()) {
      return status;
    }

    entry.type = static_cast<GgufMetadataValueType>(raw_type);

    if (entry.type == GgufMetadataValueType::kUint32) {
      // uint32 metadata，例如 alignment 这类小整数配置。
      std::uint32_t value = 0;
      status = ReadUint32(input, &value);
      entry.value = value;
    } else if (entry.type == GgufMetadataValueType::kUint64) {
      // uint64 metadata，例如较大的计数或类型字段。
      std::uint64_t value = 0;
      status = ReadUint64(input, &value);
      entry.value = value;
    } else if (entry.type == GgufMetadataValueType::kBool) {
      // bool 在 GGUF 中按 1 字节存储，只允许 0 或 1。
      std::uint8_t value = 0;
      status = ReadUint8(input, &value);
      if (status.ok() && value > 1) {
        return Status::InvalidArgument("GGUF bool value is invalid");
      }
      entry.value = value != 0;
    } else if (entry.type == GgufMetadataValueType::kString) {
      // string metadata，例如 general.architecture。
      std::string value;
      status = ReadString(input, &value);
      entry.value = std::move(value);
    } else {
      // 待扩展：array、int、float 等类型需要知道准确布局后再支持。
      return Status::InvalidArgument(
          "Unsupported GGUF metadata value type");
    }

    if (!status.ok()) {
      return status;
    }

    metadata->push_back(std::move(entry));
  }

  return Status::Ok();
}

// 读取 tensor info 列表。调用方负责先读取 header 和 metadata。
Status ReadTensorInfoEntries(std::istream* input,
                             std::uint64_t tensor_count,
                             std::vector<GgufTensorInfo>* tensor_infos) {
  tensor_infos->clear();

  // i 是 tensor info 条目索引，数量来自 header.tensor_count。
  for (std::uint64_t i = 0; i < tensor_count; ++i) {
    // info 保存当前 tensor 的结构信息，不保存权重数据。
    GgufTensorInfo info{};

    Status status = ReadString(input, &info.name);
    if (!status.ok()) {
      return status;
    }

    // dimension_count 是当前 tensor 的维度数量。
    std::uint32_t dimension_count = 0;
    status = ReadUint32(input, &dimension_count);
    if (!status.ok()) {
      return status;
    }

    if (dimension_count == 0 ||
        dimension_count > kMaxTensorDimensions) {
      return Status::InvalidArgument(
          "GGUF tensor dimension count is invalid");
    }

    info.dimensions.reserve(dimension_count);

    // dim_index 是当前读取的维度下标。
    for (std::uint32_t dim_index = 0; dim_index < dimension_count;
         ++dim_index) {
      std::uint64_t dimension = 0;
      status = ReadUint64(input, &dimension);
      if (!status.ok()) {
        return status;
      }

      if (dimension == 0) {
        return Status::InvalidArgument(
            "GGUF tensor dimension size is invalid");
      }

      info.dimensions.push_back(dimension);
    }

    // raw_type 是 GGML tensor type 编号；当前只保存，不解释 block 布局。
    std::uint32_t raw_type = 0;
    status = ReadUint32(input, &raw_type);
    if (!status.ok()) {
      return status;
    }
    info.type = static_cast<GgufTensorType>(raw_type);

    status = ReadUint64(input, &info.offset);
    if (!status.ok()) {
      return status;
    }

    tensor_infos->push_back(std::move(info));
  }

  return Status::Ok();
}

}  // namespace

// 构造 reader 只保存路径，不立即读取文件。
GgufReader::GgufReader(std::string path)
    : path_(std::move(path)), header_{}, data_section_offset_(0) {}

// 返回 reader 持有的路径。
const std::string& GgufReader::path() const {
  return path_;
}

// 返回最近一次成功读取到的 header。
const GgufHeader& GgufReader::header() const {
  return header_;
}

// 返回最近一次成功读取到的 metadata 列表。
const std::vector<GgufMetadataEntry>& GgufReader::metadata() const {
  return metadata_;
}

// 返回最近一次成功读取到的 tensor info 列表。
const std::vector<GgufTensorInfo>& GgufReader::tensor_infos() const {
  return tensor_infos_;
}

// 返回最近一次完整读取出的 data section 起点。
std::uint64_t GgufReader::data_section_offset() const {
  return data_section_offset_;
}

// 只读取 header；metadata_ 不会被修改。
Status GgufReader::read_header() {
  // input 是当前 GGUF 文件的二进制输入流。
  std::ifstream input;
  // status 保存每一步读取/校验的结果。
  Status status = OpenInput(path_, &input);
  if (!status.ok()) {
    return status;
  }

  // header 是临时结果，完整读取成功后才写入成员变量。
  GgufHeader header{};
  status = ReadHeader(&input, &header);
  if (!status.ok()) {
    return status;
  }

  header_ = header;
  return Status::Ok();
}

// 读取 header 和 metadata；暂不读取 tensor info 或权重数据。
Status GgufReader::read_metadata() {
  // input 是当前 GGUF 文件的二进制输入流。
  std::ifstream input;
  // status 保存每一步读取/校验的结果。
  Status status = OpenInput(path_, &input);
  if (!status.ok()) {
    return status;
  }

  // header 是临时结果，metadata 读取成功后才写入成员变量。
  GgufHeader header{};
  status = ReadHeader(&input, &header);
  if (!status.ok()) {
    return status;
  }

  // metadata 是临时列表，避免读取到一半失败时污染旧状态。
  std::vector<GgufMetadataEntry> metadata;
  status = ReadMetadataEntries(
      &input, header.metadata_kv_count, &metadata);
  if (!status.ok()) {
    return status;
  }

  // 所有 metadata 都读取成功后，再提交到成员变量。
  header_ = header;
  metadata_ = std::move(metadata);
  tensor_infos_.clear();
  data_section_offset_ = 0;
  return Status::Ok();
}

// 读取 header、metadata 和 tensor info，并计算数据区与权重数据偏移。
Status GgufReader::read_tensor_infos() {
  // input 是当前 GGUF 文件的二进制输入流。
  std::ifstream input;
  // status 保存每一步读取/校验的结果。
  Status status = OpenInput(path_, &input);
  if (!status.ok()) {
    return status;
  }

  // header 是临时结果，完整读取成功后才写入成员变量。
  GgufHeader header{};
  status = ReadHeader(&input, &header);
  if (!status.ok()) {
    return status;
  }

  // metadata 和 tensor_infos 都使用临时容器，保证失败时对象仍保持旧状态。
  std::vector<GgufMetadataEntry> metadata;
  status = ReadMetadataEntries(
      &input, header.metadata_kv_count, &metadata);
  if (!status.ok()) {
    return status;
  }

  std::vector<GgufTensorInfo> tensor_infos;
  status = ReadTensorInfoEntries(
      &input, header.tensor_count, &tensor_infos);
  if (!status.ok()) {
    return status;
  }

  // unaligned_data_section_offset 是 tensor info 结束后的当前位置，后面可能还有 padding。
  std::uint64_t unaligned_data_section_offset = 0;
  status = CurrentStreamOffset(&input, &unaligned_data_section_offset);
  if (!status.ok()) {
    return status;
  }

  // alignment 来自 general.alignment，缺省时使用 GGUF 默认 32 字节。
  std::uint64_t alignment = 0;
  status = GetDataAlignment(metadata, &alignment);
  if (!status.ok()) {
    return status;
  }

  // data_section_offset 是真正可以开始读取 tensor 数据的文件绝对位置。
  std::uint64_t data_section_offset = 0;
  status = AlignOffset(
      unaligned_data_section_offset, alignment, &data_section_offset);
  if (!status.ok()) {
    return status;
  }

  status = FillTensorDataOffsets(data_section_offset, &tensor_infos);
  if (!status.ok()) {
    return status;
  }

  // header、metadata、tensor info 和偏移计算全部成功后再一次性提交。
  header_ = header;
  metadata_ = std::move(metadata);
  tensor_infos_ = std::move(tensor_infos);
  data_section_offset_ = data_section_offset;
  return Status::Ok();
}

}  // namespace firstllm
