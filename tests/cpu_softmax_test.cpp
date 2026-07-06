#include "firstllm/kernels/cpu/softmax.h"

#include <cassert>
#include <cmath>
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

bool Near(float lhs, float rhs, float epsilon = 1e-5f) {
  return std::fabs(lhs - rhs) <= epsilon;
}

}  // namespace

int main() {
  firstllm::Tensor input(firstllm::DType::kFloat32,
                         firstllm::TensorShape({2, 3}));
  firstllm::Tensor output(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 3}));

  WriteFloat(&input, 0, 1.0f);
  WriteFloat(&input, 1, 2.0f);
  WriteFloat(&input, 2, 3.0f);

  WriteFloat(&input, 3, 1000.0f);
  WriteFloat(&input, 4, 1000.0f);
  WriteFloat(&input, 5, 1000.0f);

  const firstllm::Status ok =
      firstllm::CpuSoftmaxLastDim(input, &output);

  assert(ok.ok());

  assert(Near(ReadFloat(output, 0), 0.09003057f));
  assert(Near(ReadFloat(output, 1), 0.24472848f));
  assert(Near(ReadFloat(output, 2), 0.66524094f));

  assert(Near(ReadFloat(output, 3), 1.0f / 3.0f));
  assert(Near(ReadFloat(output, 4), 1.0f / 3.0f));
  assert(Near(ReadFloat(output, 5), 1.0f / 3.0f));

  const float row0_sum =
      ReadFloat(output, 0) + ReadFloat(output, 1) + ReadFloat(output, 2);
  const float row1_sum =
      ReadFloat(output, 3) + ReadFloat(output, 4) + ReadFloat(output, 5);

  assert(Near(row0_sum, 1.0f));
  assert(Near(row1_sum, 1.0f));

  const firstllm::Status null_output =
      firstllm::CpuSoftmaxLastDim(input, nullptr);

  assert(!null_output.ok());
  assert(null_output.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 3}));

  const firstllm::Status dtype_error =
      firstllm::CpuSoftmaxLastDim(wrong_dtype, &output);

  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_shape(firstllm::DType::kFloat32,
                               firstllm::TensorShape({3, 2}));

  const firstllm::Status shape_error =
      firstllm::CpuSoftmaxLastDim(input, &wrong_shape);

  assert(!shape_error.ok());
  assert(shape_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "cpu_softmax_test passed\n";
  return 0;
}