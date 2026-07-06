#include "firstllm/kernels/cpu/add.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace {

void WriteFloat(firstllm::Tensor* tensor, std::size_t index, float value) {
  std::memcpy(tensor->data() + index * sizeof(float),
              &value,
              sizeof(float));
}

float ReadFloat(const firstllm::Tensor& tensor, std::size_t index) {
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

}  // namespace

int main() {
  firstllm::Tensor lhs(firstllm::DType::kFloat32,
                       firstllm::TensorShape({2, 2}));
  firstllm::Tensor rhs(firstllm::DType::kFloat32,
                       firstllm::TensorShape({2, 2}));
  firstllm::Tensor output(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 2}));

  WriteFloat(&lhs, 0, 1.0f);
  WriteFloat(&lhs, 1, 2.0f);
  WriteFloat(&lhs, 2, 3.0f);
  WriteFloat(&lhs, 3, 4.0f);

  WriteFloat(&rhs, 0, 10.0f);
  WriteFloat(&rhs, 1, 20.0f);
  WriteFloat(&rhs, 2, 30.0f);
  WriteFloat(&rhs, 3, 40.0f);

  const firstllm::Status ok = firstllm::CpuAdd(lhs, rhs, &output);

  assert(ok.ok());
  assert(ReadFloat(output, 0) == 11.0f);
  assert(ReadFloat(output, 1) == 22.0f);
  assert(ReadFloat(output, 2) == 33.0f);
  assert(ReadFloat(output, 3) == 44.0f);

  const firstllm::Status null_output =
      firstllm::CpuAdd(lhs, rhs, nullptr);

  assert(!null_output.ok());
  assert(null_output.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 2}));

  const firstllm::Status dtype_error =
      firstllm::CpuAdd(wrong_dtype, rhs, &output);

  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_shape(firstllm::DType::kFloat32,
                               firstllm::TensorShape({4}));

  const firstllm::Status shape_error =
      firstllm::CpuAdd(wrong_shape, rhs, &output);

  assert(!shape_error.ok());
  assert(shape_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "cpu_add_test passed\n";
  return 0;
}