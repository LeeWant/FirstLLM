#include "firstllm/kernels/cpu/softmax.h"

#include <cmath>
#include <cstring>
#include <limits>

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

Status CpuSoftmaxLastDim(const Tensor& input, Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuSoftmaxLastDim output tensor is null");
  }

  if (input.dtype() != DType::kFloat32 ||
      output->dtype() != DType::kFloat32) {
    return Status::InvalidArgument(
        "CpuSoftmaxLastDim only supports float32 tensors");
  }

  if (input.shape().dims() != output->shape().dims()) {
    return Status::InvalidArgument(
        "CpuSoftmaxLastDim requires matching shapes");
  }

  if (input.shape().rank() == 0) {
    return Status::InvalidArgument(
        "CpuSoftmaxLastDim requires at least 1 dimension");
  }

  const std::size_t last_dim = input.shape().dim(input.shape().rank() - 1);
  if (last_dim == 0) {
    return Status::InvalidArgument(
        "CpuSoftmaxLastDim last dimension must be non-zero");
  }

  const std::size_t row_count = input.num_elements() / last_dim;

  for (std::size_t row = 0; row < row_count; ++row) {
    const std::size_t row_offset = row * last_dim;

    float max_value = -std::numeric_limits<float>::infinity();

    for (std::size_t col = 0; col < last_dim; ++col) {
      const float value = ReadFloat(input, row_offset + col);
      if (value > max_value) {
        max_value = value;
      }
    }

    float sum = 0.0f;

    for (std::size_t col = 0; col < last_dim; ++col) {
      const float shifted = ReadFloat(input, row_offset + col) - max_value;
      const float exp_value = std::exp(shifted);
      WriteFloat(output, row_offset + col, exp_value);
      sum += exp_value;
    }

    for (std::size_t col = 0; col < last_dim; ++col) {
      const float value = ReadFloat(*output, row_offset + col) / sum;
      WriteFloat(output, row_offset + col, value);
    }
  }

  return Status::Ok();
}

}  // namespace firstllm