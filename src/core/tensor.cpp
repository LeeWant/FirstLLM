#include "firstllm/core/tensor.h"

#include <utility>

namespace firstllm {

std::size_t DTypeSize(DType dtype) {
  switch (dtype) {
    case DType::kFloat32:
      return 4;
    case DType::kInt32:
      return 4;
    case DType::kUInt8:
      return 1;
  }

  return 0;
}

TensorShape::TensorShape() : dims_() {}

TensorShape::TensorShape(std::vector<std::size_t> dims)
    : dims_(std::move(dims)) {}

const std::vector<std::size_t>& TensorShape::dims() const {
  return dims_;
}

std::size_t TensorShape::rank() const {
  return dims_.size();
}

std::size_t TensorShape::dim(std::size_t index) const {
  return dims_.at(index);
}

std::size_t TensorShape::num_elements() const {
  std::size_t count = 1;

  for (std::size_t dim : dims_) {
    count *= dim;
  }

  return count;
}

Tensor::Tensor(DType dtype, TensorShape shape)
    : dtype_(dtype),
      shape_(std::move(shape)),
      data_(shape_.num_elements() * DTypeSize(dtype_)) {}

DType Tensor::dtype() const {
  return dtype_;
}

const TensorShape& Tensor::shape() const {
  return shape_;
}

std::size_t Tensor::num_elements() const {
  return shape_.num_elements();
}

std::size_t Tensor::byte_size() const {
  return data_.size();
}

std::uint8_t* Tensor::data() {
  return data_.data();
}

const std::uint8_t* Tensor::data() const {
  return data_.data();
}

}  // namespace firstllm