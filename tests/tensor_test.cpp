#include "firstllm/core/tensor.h"

#include <cassert>
#include <iostream>

int main() {
  assert(firstllm::DTypeSize(firstllm::DType::kFloat32) == 4);
  assert(firstllm::DTypeSize(firstllm::DType::kInt32) == 4);
  assert(firstllm::DTypeSize(firstllm::DType::kUInt8) == 1);

  const firstllm::TensorShape shape({2, 3});

  assert(shape.rank() == 2);
  assert(shape.dim(0) == 2);
  assert(shape.dim(1) == 3);
  assert(shape.num_elements() == 6);

  const firstllm::Tensor tensor(firstllm::DType::kFloat32, shape);

  assert(tensor.dtype() == firstllm::DType::kFloat32);
  assert(tensor.shape().rank() == 2);
  assert(tensor.num_elements() == 6);
  assert(tensor.byte_size() == 24);
  assert(tensor.data() != nullptr);

  const firstllm::Tensor bytes(firstllm::DType::kUInt8,
                                firstllm::TensorShape({4}));

  assert(bytes.num_elements() == 4);
  assert(bytes.byte_size() == 4);

  std::cout << "tensor_test passed\n";
  return 0;
}