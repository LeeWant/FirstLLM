#include "firstllm/kernels/cpu/rms_norm.h"

#include <cmath>
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

Status CpuRmsNorm(const Tensor& input,
                  const Tensor& weight,
                  float epsilon,
                  Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuRmsNorm output tensor is null");
  }

  if (input.dtype() != DType::kFloat32 ||
      weight.dtype() != DType::kFloat32 ||
      output->dtype() != DType::kFloat32) {
    return Status::InvalidArgument("CpuRmsNorm only supports float32 tensors");
  }

  if (input.shape().rank() != 2) {
    return Status::InvalidArgument("CpuRmsNorm input must be 2D");
  }

  if (weight.shape().rank() != 1) {
    return Status::InvalidArgument("CpuRmsNorm weight must be 1D");
  }

  if (output->shape().dims() != input.shape().dims()) {
    return Status::InvalidArgument("CpuRmsNorm output shape must match input");
  }

  const std::size_t rows = input.shape().dim(0);
  const std::size_t hidden_size = input.shape().dim(1);

  if (weight.shape().dim(0) != hidden_size) {
    return Status::InvalidArgument(
        "CpuRmsNorm weight size must match hidden size");
  }

  if (hidden_size == 0) {
    return Status::InvalidArgument("CpuRmsNorm hidden size must be non-zero");
  }

  for (std::size_t row = 0; row < rows; ++row) {
    const std::size_t row_offset = row * hidden_size;

    float mean_square = 0.0f;

    for (std::size_t col = 0; col < hidden_size; ++col) {
      const float value = ReadFloat(input, row_offset + col);
      mean_square += value * value;
    }

    mean_square /= static_cast<float>(hidden_size);

    const float scale = 1.0f / std::sqrt(mean_square + epsilon);

    for (std::size_t col = 0; col < hidden_size; ++col) {
      const float value = ReadFloat(input, row_offset + col);
      const float weight_value = ReadFloat(weight, col);
      const float output_value = value * scale * weight_value;

      WriteFloat(output, row_offset + col, output_value);
    }
  }

  return Status::Ok();
}

}  // namespace firstllm