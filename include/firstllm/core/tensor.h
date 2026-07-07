#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace firstllm {

// 张量元素类型。当前只保留最小集合，后续可扩展 float16、bfloat16、量化类型等。
enum class DType {
  kFloat32,  // 32 位浮点数，当前 CPU 算子的主要计算类型。
  kInt32,    // 32 位整数，当前主要用于测试和未来索引/配置类数据。
  kUInt8,    // 8 位无符号整数，可用于字节数据或未来量化权重。
};

// 返回一个 dtype 对应的单元素字节数；未知 dtype 当前返回 0。
std::size_t DTypeSize(DType dtype);

// TensorShape 只保存维度列表，不保存 stride、layout 或 view 信息。
class TensorShape {
 public:
  // 默认构造空 shape；当前约定空 shape 的 num_elements() 为 1，可表示 scalar。
  TensorShape();

  // 用维度数组构造 shape，例如 {2, 3} 表示 2 行 3 列。
  explicit TensorShape(std::vector<std::size_t> dims);

  // 返回完整维度列表。
  const std::vector<std::size_t>& dims() const;

  // 返回 rank，也就是维度个数。
  std::size_t rank() const;

  // 返回第 index 个维度；index 越界时由 vector::at 抛出异常。
  std::size_t dim(std::size_t index) const;

  // 返回元素总数，即所有维度的乘积。
  std::size_t num_elements() const;

 private:
  std::vector<std::size_t> dims_;  // 每一维的大小，按 row-major 语义理解。
};

// Tensor 是第一版最小数据容器：dtype + shape + host raw bytes。
class Tensor {
 public:
  // 构造时按 dtype 和 shape 分配 host 内存；暂不支持外部内存或 GPU 内存。
  Tensor(DType dtype, TensorShape shape);

  // 返回元素类型。
  DType dtype() const;

  // 返回 shape 对象。
  const TensorShape& shape() const;

  // 返回元素数量。
  std::size_t num_elements() const;

  // 返回底层字节数。
  std::size_t byte_size() const;

  // 返回可写 raw byte 指针；算子用 memcpy 读写具体类型。
  std::uint8_t* data();

  // 返回只读 raw byte 指针。
  const std::uint8_t* data() const;

 private:
  DType dtype_;                    // 元素类型，决定每个元素占多少字节。
  TensorShape shape_;              // 形状信息，决定元素数量和索引语义。
  std::vector<std::uint8_t> data_;  // 实际 host 内存，当前不保存类型化指针。
};

}  // namespace firstllm
