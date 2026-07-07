#include "firstllm/core/tensor.h"

#include <cassert>
#include <iostream>

int main() {
  // 验证 dtype 到字节数的映射，这是 tensor 内存分配的基础。
  assert(firstllm::DTypeSize(firstllm::DType::kFloat32) == 4);
  assert(firstllm::DTypeSize(firstllm::DType::kInt32) == 4);
  assert(firstllm::DTypeSize(firstllm::DType::kUInt8) == 1);

  // shape 表示一个 2x3 的二维张量。
  const firstllm::TensorShape shape({2, 3});

  assert(shape.rank() == 2);
  assert(shape.dim(0) == 2);
  assert(shape.dim(1) == 3);
  assert(shape.num_elements() == 6);

  // tensor 是 float32 + 2x3，因此应分配 6 * 4 = 24 字节。
  const firstllm::Tensor tensor(firstllm::DType::kFloat32, shape);

  assert(tensor.dtype() == firstllm::DType::kFloat32);
  assert(tensor.shape().rank() == 2);
  assert(tensor.num_elements() == 6);
  assert(tensor.byte_size() == 24);
  assert(tensor.data() != nullptr);

  // bytes 验证 uint8 的单元素字节数是 1。
  const firstllm::Tensor bytes(firstllm::DType::kUInt8,
                               firstllm::TensorShape({4}));

  assert(bytes.num_elements() == 4);
  assert(bytes.byte_size() == 4);

  std::cout << "tensor_test passed\n";
  return 0;
}
