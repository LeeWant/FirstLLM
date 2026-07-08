#include "firstllm/model/gguf_reader.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <variant>
#include <vector>

namespace {

// GGUF 文件魔数，写入文件时按 little-endian 输出。
constexpr std::uint32_t kGgufMagic = 0x46554747U;

// 写入 1 字节整数，当前用于 bool metadata。
void WriteUint8(std::ostream* output, std::uint8_t value) {
  output->put(static_cast<char>(value));
}

// 按 little-endian 写入 uint32。
void WriteUint32(std::ostream* output, std::uint32_t value) {
  for (int i = 0; i < 4; ++i) {
    output->put(static_cast<char>((value >> (8 * i)) & 0xff));
  }
}

// 按 little-endian 写入 uint64。
void WriteUint64(std::ostream* output, std::uint64_t value) {
  for (int i = 0; i < 8; ++i) {
    output->put(static_cast<char>((value >> (8 * i)) & 0xff));
  }
}

// 测试用 offset 对齐函数，和 reader 里的实现保持同一语义。
std::uint64_t AlignUpForTest(std::uint64_t offset,
                             std::uint64_t alignment) {
  const std::uint64_t remainder = offset % alignment;
  if (remainder == 0) {
    return offset;
  }

  return offset + (alignment - remainder);
}

// 写入 padding，把后续模拟权重数据推到 data section 起点。
std::uint64_t WritePaddingToAlignment(std::ostream* output,
                                      std::uint64_t alignment) {
  const std::streamoff raw_offset =
      static_cast<std::streamoff>(output->tellp());
  assert(raw_offset >= 0);

  const std::uint64_t offset = static_cast<std::uint64_t>(raw_offset);
  const std::uint64_t aligned_offset = AlignUpForTest(offset, alignment);

  // padding_count 是 tensor info 结束到 data section 起点之间的填充字节数。
  const std::uint64_t padding_count = aligned_offset - offset;
  for (std::uint64_t i = 0; i < padding_count; ++i) {
    WriteUint8(output, 0);
  }

  return aligned_offset;
}

// 写入 GGUF string：长度 + 原始字节。
void WriteString(std::ostream* output, const std::string& value) {
  WriteUint64(output, static_cast<std::uint64_t>(value.size()));
  output->write(value.data(), static_cast<std::streamsize>(value.size()));
}

// 写入最小 GGUF header。
void WriteHeader(std::ostream* output,
                 std::uint32_t magic,
                 std::uint32_t version,
                 std::uint64_t tensor_count,
                 std::uint64_t metadata_kv_count) {
  WriteUint32(output, magic);
  WriteUint32(output, version);
  WriteUint64(output, tensor_count);
  WriteUint64(output, metadata_kv_count);
}

// 创建只包含 header 的 GGUF-like 测试文件。
void WriteHeaderFile(const std::filesystem::path& path,
                     std::uint32_t magic,
                     std::uint32_t version,
                     std::uint64_t tensor_count,
                     std::uint64_t metadata_kv_count) {
  std::ofstream output(path, std::ios::binary);

  assert(output);

  WriteHeader(&output, magic, version, tensor_count, metadata_kv_count);
}

// 写入 uint32 metadata 条目。
void WriteMetadataUint32(std::ostream* output,
                         const std::string& key,
                         std::uint32_t value) {
  WriteString(output, key);
  WriteUint32(
      output,
      static_cast<std::uint32_t>(firstllm::GgufMetadataValueType::kUint32));
  WriteUint32(output, value);
}

// 写入 uint64 metadata 条目。
void WriteMetadataUint64(std::ostream* output,
                         const std::string& key,
                         std::uint64_t value) {
  WriteString(output, key);
  WriteUint32(
      output,
      static_cast<std::uint32_t>(firstllm::GgufMetadataValueType::kUint64));
  WriteUint64(output, value);
}

// 写入 bool metadata 条目，value 应为 0 或 1。
void WriteMetadataBool(std::ostream* output,
                       const std::string& key,
                       std::uint8_t value) {
  WriteString(output, key);
  WriteUint32(
      output,
      static_cast<std::uint32_t>(firstllm::GgufMetadataValueType::kBool));
  WriteUint8(output, value);
}

// 写入 string metadata 条目。
void WriteMetadataString(std::ostream* output,
                         const std::string& key,
                         const std::string& value) {
  WriteString(output, key);
  WriteUint32(
      output,
      static_cast<std::uint32_t>(firstllm::GgufMetadataValueType::kString));
  WriteString(output, value);
}

// 写入 tensor info：name + n_dimensions + dimensions + type + offset。
void WriteTensorInfo(std::ostream* output,
                     const std::string& name,
                     const std::vector<std::uint64_t>& dimensions,
                     firstllm::GgufTensorType type,
                     std::uint64_t offset) {
  WriteString(output, name);
  WriteUint32(output, static_cast<std::uint32_t>(dimensions.size()));

  // dimension 是当前写入的单个维度大小。
  for (const std::uint64_t dimension : dimensions) {
    WriteUint64(output, dimension);
  }

  WriteUint32(output, static_cast<std::uint32_t>(type));
  WriteUint64(output, offset);
}

}  // namespace

int main() {
  // temp_dir 是测试临时文件目录。
  const std::filesystem::path temp_dir =
      std::filesystem::temp_directory_path();

  // valid_path 是只包含 header 的正常测试文件。
  const std::filesystem::path valid_path =
      temp_dir / "firstllm_valid_header.gguf";

  WriteHeaderFile(valid_path, kGgufMagic, 3, 2, 5);

  // reader 用于验证 read_header()。
  firstllm::GgufReader reader(valid_path.string());

  const firstllm::Status ok = reader.read_header();

  assert(ok.ok());
  assert(reader.path() == valid_path.string());
  assert(reader.header().magic == kGgufMagic);
  assert(reader.header().version == 3);
  assert(reader.header().tensor_count == 2);
  assert(reader.header().metadata_kv_count == 5);

  // metadata_path 是包含 4 条 metadata 的正常测试文件。
  const std::filesystem::path metadata_path =
      temp_dir / "firstllm_metadata.gguf";

  {
    std::ofstream output(metadata_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 0, 4);
    WriteMetadataString(&output, "general.architecture", "llama");
    WriteMetadataUint32(&output, "general.alignment", 32);
    WriteMetadataUint64(&output, "general.file_type", 1);
    WriteMetadataBool(&output, "firstllm.has_metadata", 1);
  }

  // metadata_reader 用于验证 read_metadata() 成功路径。
  firstllm::GgufReader metadata_reader(metadata_path.string());

  const firstllm::Status metadata_status =
      metadata_reader.read_metadata();

  assert(metadata_status.ok());
  assert(metadata_reader.header().metadata_kv_count == 4);
  assert(metadata_reader.metadata().size() == 4);

  // 第 0 条验证 string metadata。
  assert(metadata_reader.metadata()[0].key == "general.architecture");
  assert(metadata_reader.metadata()[0].type ==
         firstllm::GgufMetadataValueType::kString);
  assert(std::get<std::string>(metadata_reader.metadata()[0].value) ==
         "llama");

  // 第 1 条验证 uint32 metadata。
  assert(metadata_reader.metadata()[1].key == "general.alignment");
  assert(metadata_reader.metadata()[1].type ==
         firstllm::GgufMetadataValueType::kUint32);
  assert(std::get<std::uint32_t>(metadata_reader.metadata()[1].value) == 32);

  // 第 2 条验证 uint64 metadata。
  assert(metadata_reader.metadata()[2].key == "general.file_type");
  assert(metadata_reader.metadata()[2].type ==
         firstllm::GgufMetadataValueType::kUint64);
  assert(std::get<std::uint64_t>(metadata_reader.metadata()[2].value) == 1);

  // 第 3 条验证 bool metadata。
  assert(metadata_reader.metadata()[3].key == "firstllm.has_metadata");
  assert(metadata_reader.metadata()[3].type ==
         firstllm::GgufMetadataValueType::kBool);
  assert(std::get<bool>(metadata_reader.metadata()[3].value));

  // tensor_info_path 是包含 metadata 和 2 条 tensor info 的正常测试文件。
  const std::filesystem::path tensor_info_path =
      temp_dir / "firstllm_tensor_info.gguf";

  // expected_data_section_offset 用于验证 reader 计算出的数据区起点。
  std::uint64_t expected_data_section_offset = 0;

  {
    std::ofstream output(tensor_info_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 2, 1);
    WriteMetadataString(&output, "general.architecture", "llama");
    WriteTensorInfo(&output,
                    "token_embd.weight",
                    {32000, 4096},
                    firstllm::GgufTensorType::kFloat16,
                    0);
    WriteTensorInfo(&output,
                    "output.weight",
                    {4096, 32000},
                    firstllm::GgufTensorType::kFloat32,
                    262144);

    expected_data_section_offset = WritePaddingToAlignment(&output, 32);

    // 模拟真实 GGUF 中 data section 后的权重字节；reader 当前只定位，不读取。
    WriteUint8(&output, 0xaa);
    WriteUint8(&output, 0xbb);
  }

  // tensor_info_reader 验证 read_tensor_infos() 成功路径。
  firstllm::GgufReader tensor_info_reader(tensor_info_path.string());

  const firstllm::Status tensor_info_status =
      tensor_info_reader.read_tensor_infos();

  assert(tensor_info_status.ok());
  assert(tensor_info_reader.header().tensor_count == 2);
  assert(tensor_info_reader.header().metadata_kv_count == 1);
  assert(tensor_info_reader.metadata().size() == 1);
  assert(tensor_info_reader.tensor_infos().size() == 2);
  assert(tensor_info_reader.data_section_offset() ==
         expected_data_section_offset);

  // 第 0 条验证 embedding 权重的名称、形状、类型、相对 offset 和绝对 offset。
  assert(tensor_info_reader.tensor_infos()[0].name ==
         "token_embd.weight");
  assert(tensor_info_reader.tensor_infos()[0].dimensions.size() == 2);
  assert(tensor_info_reader.tensor_infos()[0].dimensions[0] == 32000);
  assert(tensor_info_reader.tensor_infos()[0].dimensions[1] == 4096);
  assert(tensor_info_reader.tensor_infos()[0].type ==
         firstllm::GgufTensorType::kFloat16);
  assert(tensor_info_reader.tensor_infos()[0].offset == 0);
  assert(tensor_info_reader.tensor_infos()[0].data_offset ==
         expected_data_section_offset);

  // 第 1 条验证输出权重的结构信息和绝对文件偏移。
  assert(tensor_info_reader.tensor_infos()[1].name == "output.weight");
  assert(tensor_info_reader.tensor_infos()[1].dimensions.size() == 2);
  assert(tensor_info_reader.tensor_infos()[1].dimensions[0] == 4096);
  assert(tensor_info_reader.tensor_infos()[1].dimensions[1] == 32000);
  assert(tensor_info_reader.tensor_infos()[1].type ==
         firstllm::GgufTensorType::kFloat32);
  assert(tensor_info_reader.tensor_infos()[1].offset == 262144);
  assert(tensor_info_reader.tensor_infos()[1].data_offset ==
         expected_data_section_offset + 262144);

  // custom_alignment_path 验证 general.alignment 会覆盖默认 32 字节对齐。
  const std::filesystem::path custom_alignment_path =
      temp_dir / "firstllm_custom_alignment.gguf";

  std::uint64_t expected_custom_data_section_offset = 0;

  {
    std::ofstream output(custom_alignment_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 1, 1);
    WriteMetadataUint32(&output, "general.alignment", 64);
    WriteTensorInfo(&output,
                    "custom.weight",
                    {4, 4},
                    firstllm::GgufTensorType::kFloat32,
                    16);

    expected_custom_data_section_offset =
        WritePaddingToAlignment(&output, 64);
    WriteUint8(&output, 0xcc);
  }

  firstllm::GgufReader custom_alignment_reader(
      custom_alignment_path.string());

  const firstllm::Status custom_alignment_status =
      custom_alignment_reader.read_tensor_infos();

  assert(custom_alignment_status.ok());
  assert(custom_alignment_reader.data_section_offset() ==
         expected_custom_data_section_offset);
  assert(custom_alignment_reader.tensor_infos().size() == 1);
  assert(custom_alignment_reader.tensor_infos()[0].offset == 16);
  assert(custom_alignment_reader.tensor_infos()[0].data_offset ==
         expected_custom_data_section_offset + 16);

  // empty_path_reader 验证空路径错误。
  firstllm::GgufReader empty_path_reader("");

  const firstllm::Status empty_path_status =
      empty_path_reader.read_header();

  assert(!empty_path_status.ok());
  assert(empty_path_status.code() == firstllm::ErrorCode::kInvalidArgument);

  // missing_reader 验证文件不存在错误。
  firstllm::GgufReader missing_reader(
      (temp_dir / "firstllm_missing_file.gguf").string());

  const firstllm::Status missing_status = missing_reader.read_header();

  assert(!missing_status.ok());
  assert(missing_status.code() == firstllm::ErrorCode::kNotFound);

  // wrong_magic_path 验证 magic 不匹配错误。
  const std::filesystem::path wrong_magic_path =
      temp_dir / "firstllm_wrong_magic.gguf";

  WriteHeaderFile(wrong_magic_path, 0x12345678U, 3, 0, 0);

  firstllm::GgufReader wrong_magic_reader(wrong_magic_path.string());

  const firstllm::Status wrong_magic_status =
      wrong_magic_reader.read_header();

  assert(!wrong_magic_status.ok());
  assert(wrong_magic_status.code() == firstllm::ErrorCode::kInvalidArgument);

  // truncated_path 验证文件截断错误。
  const std::filesystem::path truncated_path =
      temp_dir / "firstllm_truncated.gguf";

  {
    std::ofstream output(truncated_path, std::ios::binary);
    assert(output);
    WriteUint32(&output, kGgufMagic);
  }

  firstllm::GgufReader truncated_reader(truncated_path.string());

  const firstllm::Status truncated_status =
      truncated_reader.read_header();

  assert(!truncated_status.ok());
  assert(truncated_status.code() == firstllm::ErrorCode::kInvalidArgument);

  // unsupported_path 验证暂不支持的 metadata type 会明确失败。
  const std::filesystem::path unsupported_path =
      temp_dir / "firstllm_unsupported_metadata.gguf";

  {
    std::ofstream output(unsupported_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 0, 1);
    WriteString(&output, "unsupported");
    WriteUint32(
        &output,
        static_cast<std::uint32_t>(firstllm::GgufMetadataValueType::kArray));
  }

  firstllm::GgufReader unsupported_reader(unsupported_path.string());

  const firstllm::Status unsupported_status =
      unsupported_reader.read_metadata();

  assert(!unsupported_status.ok());
  assert(unsupported_status.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // invalid_bool_path 验证 bool 原始值只能是 0 或 1。
  const std::filesystem::path invalid_bool_path =
      temp_dir / "firstllm_invalid_bool.gguf";

  {
    std::ofstream output(invalid_bool_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 0, 1);
    WriteMetadataBool(&output, "invalid_bool", 2);
  }

  firstllm::GgufReader invalid_bool_reader(invalid_bool_path.string());

  const firstllm::Status invalid_bool_status =
      invalid_bool_reader.read_metadata();

  assert(!invalid_bool_status.ok());
  assert(invalid_bool_status.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // invalid_alignment_path 验证 general.alignment 不能是 0。
  const std::filesystem::path invalid_alignment_path =
      temp_dir / "firstllm_invalid_alignment.gguf";

  {
    std::ofstream output(invalid_alignment_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 0, 1);
    WriteMetadataUint32(&output, "general.alignment", 0);
  }

  firstllm::GgufReader invalid_alignment_reader(
      invalid_alignment_path.string());

  const firstllm::Status invalid_alignment_status =
      invalid_alignment_reader.read_tensor_infos();

  assert(!invalid_alignment_status.ok());
  assert(invalid_alignment_status.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // invalid_dimension_count_path 验证 tensor 维度数量不能超过当前上限。
  const std::filesystem::path invalid_dimension_count_path =
      temp_dir / "firstllm_invalid_dimension_count.gguf";

  {
    std::ofstream output(invalid_dimension_count_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 1, 0);
    WriteString(&output, "invalid_tensor");
    WriteUint32(&output, 5);
  }

  firstllm::GgufReader invalid_dimension_count_reader(
      invalid_dimension_count_path.string());

  const firstllm::Status invalid_dimension_count_status =
      invalid_dimension_count_reader.read_tensor_infos();

  assert(!invalid_dimension_count_status.ok());
  assert(invalid_dimension_count_status.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // truncated_tensor_info_path 验证 tensor info 读取到一半会失败。
  const std::filesystem::path truncated_tensor_info_path =
      temp_dir / "firstllm_truncated_tensor_info.gguf";

  {
    std::ofstream output(truncated_tensor_info_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 1, 0);
    WriteString(&output, "truncated_tensor");
    WriteUint32(&output, 2);
    WriteUint64(&output, 4096);
  }

  firstllm::GgufReader truncated_tensor_info_reader(
      truncated_tensor_info_path.string());

  const firstllm::Status truncated_tensor_info_status =
      truncated_tensor_info_reader.read_tensor_infos();

  assert(!truncated_tensor_info_status.ok());
  assert(truncated_tensor_info_status.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // overflow_tensor_offset_path 验证相对 offset 溢出时不会产生错误绝对偏移。
  const std::filesystem::path overflow_tensor_offset_path =
      temp_dir / "firstllm_overflow_tensor_offset.gguf";

  {
    std::ofstream output(overflow_tensor_offset_path, std::ios::binary);
    assert(output);

    WriteHeader(&output, kGgufMagic, 3, 1, 0);
    WriteTensorInfo(&output,
                    "overflow_tensor",
                    {1},
                    firstllm::GgufTensorType::kFloat32,
                    std::numeric_limits<std::uint64_t>::max());
  }

  firstllm::GgufReader overflow_tensor_offset_reader(
      overflow_tensor_offset_path.string());

  const firstllm::Status overflow_tensor_offset_status =
      overflow_tensor_offset_reader.read_tensor_infos();

  assert(!overflow_tensor_offset_status.ok());
  assert(overflow_tensor_offset_status.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // 清理测试产生的临时文件。
  std::filesystem::remove(valid_path);
  std::filesystem::remove(metadata_path);
  std::filesystem::remove(tensor_info_path);
  std::filesystem::remove(custom_alignment_path);
  std::filesystem::remove(wrong_magic_path);
  std::filesystem::remove(truncated_path);
  std::filesystem::remove(unsupported_path);
  std::filesystem::remove(invalid_bool_path);
  std::filesystem::remove(invalid_alignment_path);
  std::filesystem::remove(invalid_dimension_count_path);
  std::filesystem::remove(truncated_tensor_info_path);
  std::filesystem::remove(overflow_tensor_offset_path);

  std::cout << "gguf_reader_test passed\n";
  return 0;
}
