#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace firstllm {

enum class DType {
  kFloat32,
  kInt32,
  kUInt8,
};

std::size_t DTypeSize(DType dtype);

class TensorShape {
 public:
  TensorShape();

  explicit TensorShape(std::vector<std::size_t> dims);

  const std::vector<std::size_t>& dims() const;
  std::size_t rank() const;
  std::size_t dim(std::size_t index) const;
  std::size_t num_elements() const;

 private:
  std::vector<std::size_t> dims_;
};

class Tensor {
 public:
  Tensor(DType dtype, TensorShape shape);

  DType dtype() const;
  const TensorShape& shape() const;
  std::size_t num_elements() const;
  std::size_t byte_size() const;

  std::uint8_t* data();
  const std::uint8_t* data() const;

 private:
  DType dtype_;
  TensorShape shape_;
  std::vector<std::uint8_t> data_;
};

}  // namespace firstllm