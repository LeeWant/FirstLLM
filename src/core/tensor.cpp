#include "firstllm/core/tensor.h"

#include <utility>

namespace firstllm {

// 把逻辑 dtype 转成实际内存字节数。
std::size_t DTypeSize(DType dtype) {
  switch (dtype) {
    case DType::kFloat32:
      return 4;
    case DType::kInt32:
      return 4;
    case DType::kUInt8:
      return 1;
  }

  // 理论上 enum class 不会走到这里；保留 0 作为防御性返回。
  return 0;
}

// 空 shape 可表示 scalar。
TensorShape::TensorShape() : dims_() {}

// dims_ 保存每一维大小，使用 move 避免拷贝 vector。
TensorShape::TensorShape(std::vector<std::size_t> dims)
    : dims_(std::move(dims)) {}

// 返回原始维度数组，方便 shape 比较。
const std::vector<std::size_t>& TensorShape::dims() const {
  return dims_;
}

// rank 就是维度数量。
std::size_t TensorShape::rank() const {
  return dims_.size();
}

// 使用 at() 可以在越界时更早暴露错误。
std::size_t TensorShape::dim(std::size_t index) const {
  return dims_.at(index);
}

// 计算元素总数；空 shape 的乘积保持为 1。
std::size_t TensorShape::num_elements() const {
  // count 是累乘结果，初始为 1 才能正确表达 scalar。
  std::size_t count = 1;

  // dim 是当前正在累乘的单个维度大小。
  for (std::size_t dim : dims_) {
    count *= dim;
  }

  return count;
}

// Tensor 构造时立即分配 byte buffer。
Tensor::Tensor(DType dtype, TensorShape shape)
    : dtype_(dtype),
      shape_(std::move(shape)),
      data_(shape_.num_elements() * DTypeSize(dtype_)) {}

// 返回元素类型。
DType Tensor::dtype() const {
  return dtype_;
}

// 返回 shape 引用，避免拷贝。
const TensorShape& Tensor::shape() const {
  return shape_;
}

// 元素数量委托给 TensorShape。
std::size_t Tensor::num_elements() const {
  return shape_.num_elements();
}

// byte_size 直接等于底层 vector 的字节数。
std::size_t Tensor::byte_size() const {
  return data_.size();
}

// 返回可写字节指针，kernel 会用 memcpy 解释成具体类型。
std::uint8_t* Tensor::data() {
  return data_.data();
}

// 返回只读字节指针。
const std::uint8_t* Tensor::data() const {
  return data_.data();
}

}  // namespace firstllm
