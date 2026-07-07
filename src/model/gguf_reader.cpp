#include "firstllm/model/gguf_reader.h"

#include <array>
#include <fstream>
#include <istream>
#include <limits>
#include <utility>

namespace firstllm {
namespace {

// GGUF 文件魔数，按 little-endian 读取后等于字符序列 "GGUF"。
constexpr std::uint32_t kGgufMagic = 0x46554747U;

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

}  // namespace

// 构造 reader 只保存路径，不立即读取文件。
GgufReader::GgufReader(std::string path)
    : path_(std::move(path)), header_{} {}

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

  // i 是 metadata 条目索引，数量来自 header.metadata_kv_count。
  for (std::uint64_t i = 0; i < header.metadata_kv_count; ++i) {
    // entry 保存当前正在读取的一条 key/value。
    GgufMetadataEntry entry{};

    status = ReadString(&input, &entry.key);
    if (!status.ok()) {
      return status;
    }

    // raw_type 是文件中记录的原始类型编号。
    std::uint32_t raw_type = 0;
    status = ReadUint32(&input, &raw_type);
    if (!status.ok()) {
      return status;
    }

    entry.type = static_cast<GgufMetadataValueType>(raw_type);

    if (entry.type == GgufMetadataValueType::kUint32) {
      // uint32 metadata，例如 alignment 这类小整数配置。
      std::uint32_t value = 0;
      status = ReadUint32(&input, &value);
      entry.value = value;
    } else if (entry.type == GgufMetadataValueType::kUint64) {
      // uint64 metadata，例如较大的计数或类型字段。
      std::uint64_t value = 0;
      status = ReadUint64(&input, &value);
      entry.value = value;
    } else if (entry.type == GgufMetadataValueType::kBool) {
      // bool 在 GGUF 中按 1 字节存储，只允许 0 或 1。
      std::uint8_t value = 0;
      status = ReadUint8(&input, &value);
      if (status.ok() && value > 1) {
        return Status::InvalidArgument("GGUF bool value is invalid");
      }
      entry.value = value != 0;
    } else if (entry.type == GgufMetadataValueType::kString) {
      // string metadata，例如 general.architecture。
      std::string value;
      status = ReadString(&input, &value);
      entry.value = std::move(value);
    } else {
      // 待扩展：array、int、float 等类型需要知道准确布局后再支持。
      return Status::InvalidArgument(
          "Unsupported GGUF metadata value type");
    }

    if (!status.ok()) {
      return status;
    }

    metadata.push_back(std::move(entry));
  }

  // 所有 metadata 都读取成功后，再提交到成员变量。
  header_ = header;
  metadata_ = std::move(metadata);
  return Status::Ok();
}

}  // namespace firstllm
