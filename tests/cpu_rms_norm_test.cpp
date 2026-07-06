#include "firstllm/kernels/cpu/rms_norm.h"

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
                         firstllm::TensorShape({2, 2}));
  firstllm::Tensor weight(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2}));
  firstllm::Tensor output(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 2}));

  WriteFloat(&input, 0, 3.0f);
  WriteFloat(&input, 1, 4.0f);
  WriteFloat(&input, 2, 0.0f);
  WriteFloat(&input, 3, 2.0f);

  WriteFloat(&weight, 0, 1.0f);
  WriteFloat(&weight, 1, 2.0f);

  const firstllm::Status ok =
      firstllm::CpuRmsNorm(input, weight, 0.0f, &output);

  assert(ok.ok());

  const float row0_scale = 1.0f / std::sqrt((9.0f + 16.0f) / 2.0f);
  const float row1_scale = 1.0f / std::sqrt((0.0f + 4.0f) / 2.0f);

  assert(Near(ReadFloat(output, 0), 3.0f * row0_scale * 1.0f));
  assert(Near(ReadFloat(output, 1), 4.0f * row0_scale * 2.0f));
  assert(Near(ReadFloat(output, 2), 0.0f * row1_scale * 1.0f));
  assert(Near(ReadFloat(output, 3), 2.0f * row1_scale * 2.0f));

  const firstllm::Status null_output =
      firstllm::CpuRmsNorm(input, weight, 1e-6f, nullptr);

  assert(!null_output.ok());
  assert(null_output.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 2}));

  const firstllm::Status dtype_error =
      firstllm::CpuRmsNorm(wrong_dtype, weight, 1e-6f, &output);

  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_rank(firstllm::DType::kFloat32,
                              firstllm::TensorShape({2, 2, 1}));

  const firstllm::Status rank_error =
      firstllm::CpuRmsNorm(wrong_rank, weight, 1e-6f, &output);

  assert(!rank_error.ok());
  assert(rank_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_weight(firstllm::DType::kFloat32,
                                firstllm::TensorShape({3}));

  const firstllm::Status weight_error =
      firstllm::CpuRmsNorm(input, wrong_weight, 1e-6f, &output);

  assert(!weight_error.ok());
  assert(weight_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_output(firstllm::DType::kFloat32,
                                firstllm::TensorShape({2, 3}));

  const firstllm::Status output_error =
      firstllm::CpuRmsNorm(input, weight, 1e-6f, &wrong_output);

  assert(!output_error.ok());
  assert(output_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "cpu_rms_norm_test passed\n";
  return 0;
}