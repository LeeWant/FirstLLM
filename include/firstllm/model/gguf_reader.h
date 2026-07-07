#pragma once

#include "firstllm/core/status.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace firstllm {

// GGUF 文件头。当前读取的是 GGUF v2/v3 共有的基础 header 字段。
struct GgufHeader {
  std::uint32_t magic;              // 文件魔数，应为 "GGUF" 的 little-endian 表示。
  std::uint32_t version;            // GGUF 格式版本。
  std::uint64_t tensor_count;       // 文件中 tensor info 的数量。
  std::uint64_t metadata_kv_count;  // metadata key/value 条目数量。
};

// GGUF metadata value 的类型枚举，数值必须和 GGUF 规范保持一致。
enum class GgufMetadataValueType : std::uint32_t {
  kUint8 = 0,    // 暂未解析，后续需要时再加入 variant。
  kInt8 = 1,     // 暂未解析。
  kUint16 = 2,   // 暂未解析。
  kInt16 = 3,    // 暂未解析。
  kUint32 = 4,   // 当前已支持。
  kInt32 = 5,    // 暂未解析。
  kFloat32 = 6,  // 暂未解析。
  kBool = 7,     // 当前已支持，读取为 bool。
  kString = 8,   // 当前已支持，读取为 std::string。
  kArray = 9,    // 暂未解析，数组需要递归处理 element type。
  kUint64 = 10,  // 当前已支持。
  kInt64 = 11,   // 暂未解析。
  kFloat64 = 12, // 暂未解析。
};

// 当前第一版 metadata value 只承载最常用的几类。
// 后续扩展 int/float/array 时，需要同步扩展 read_metadata() 和测试。
using GgufMetadataValue = std::variant<
    std::uint32_t,
    std::uint64_t,
    bool,
    std::string>;

// 一条 GGUF metadata：key + value type + value。
struct GgufMetadataEntry {
  std::string key;                 // metadata 键名，例如 general.architecture。
  GgufMetadataValueType type;      // value 的 GGUF 类型。
  GgufMetadataValue value;         // value 的实际内容，当前只支持 variant 中的类型。
};

// GgufReader 负责读取 GGUF 的结构信息；当前不加载 tensor 权重数据。
class GgufReader {
 public:
  // 保存待读取文件路径；不会立即打开文件。
  explicit GgufReader(std::string path);

  // 返回文件路径。
  const std::string& path() const;

  // 返回最近一次读取到的 header。
  const GgufHeader& header() const;

  // 返回最近一次读取到的 metadata 列表。
  const std::vector<GgufMetadataEntry>& metadata() const;

  // 只读取 header，不读取 metadata 和 tensor info。
  Status read_header();

  // 读取 header 和 metadata key/value。
  // 当前待扩展：tensor info、权重数据、array/int/float metadata。
  Status read_metadata();

 private:
  std::string path_;                         // GGUF 文件路径。
  GgufHeader header_;                        // 最近一次成功读取的 header。
  std::vector<GgufMetadataEntry> metadata_;  // 最近一次成功读取的 metadata。
};

}  // namespace firstllm
