#include "firstllm/kernels/cpu/matmul.h"

#include <cstring>

namespace firstllm {
namespace {

float ReadFloat(const Tensor& tensor, std::size_t index) {
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

void WriteFloat(Tensor* tensor, std::size_t index, float value) {
  std::memcpy(tensor->data() + index * sizeof(float),
              &value,
              sizeof(float));
}

}  // namespace

Status CpuMatMul(const Tensor& lhs, const Tensor& rhs, Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuMatMul output tensor is null");
  }

  if (lhs.dtype() != DType::kFloat32 ||
      rhs.dtype() != DType::kFloat32 ||
      output->dtype() != DType::kFloat32) {
    return Status::InvalidArgument("CpuMatMul only supports float32 tensors");
  }

  if (lhs.shape().rank() != 2 ||
      rhs.shape().rank() != 2 ||
      output->shape().rank() != 2) {
    return Status::InvalidArgument("CpuMatMul requires 2D tensors");
  }

  const std::size_t m = lhs.shape().dim(0);
  const std::size_t k = lhs.shape().dim(1);
  const std::size_t rhs_k = rhs.shape().dim(0);
  const std::size_t n = rhs.shape().dim(1);

  if (k != rhs_k) {
    return Status::InvalidArgument("CpuMatMul inner dimensions do not match");
  }

  if (output->shape().dim(0) != m || output->shape().dim(1) != n) {
    return Status::InvalidArgument("CpuMatMul output shape is invalid");
  }

  for (std::size_t row = 0; row < m; ++row) {
    for (std::size_t col = 0; col < n; ++col) {
      float sum = 0.0f;

      for (std::size_t inner = 0; inner < k; ++inner) {
        const float lhs_value = ReadFloat(lhs, row * k + inner);
        const float rhs_value = ReadFloat(rhs, inner * n + col);
        sum += lhs_value * rhs_value;
      }

      WriteFloat(output, row * n + col, sum);
    }
  }

  return Status::Ok();
}

}  // namespace firstllm