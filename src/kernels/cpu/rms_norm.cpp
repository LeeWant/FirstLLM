#include "firstllm/kernels/cpu/rms_norm.h"

#include <cmath>
#include <cstring>

namespace firstllm {
namespace {

// 从 Tensor 的 raw bytes 中读取一个 float32。
float ReadFloat(const Tensor& tensor, std::size_t index) {
  // value 保存读出的 float 值。
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

// 把一个 float32 写回 Tensor 的 raw bytes。
void WriteFloat(Tensor* tensor, std::size_t index, float value) {
  std::memcpy(tensor->data() + index * sizeof(float),
              &value,
              sizeof(float));
}

}  // namespace

// RMSNorm：output[row, col] = input[row, col] * weight[col] / rms(row)。
Status CpuRmsNorm(const Tensor& input,
                  const Tensor& weight,
                  float epsilon,
                  Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuRmsNorm output tensor is null");
  }

  // 当前只支持 float32。真实 LLM 常用 half/bfloat16，后续再扩展。
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

  // rows 是 batch/token 方向，hidden_size 是每行归一化的宽度。
  const std::size_t rows = input.shape().dim(0);
  const std::size_t hidden_size = input.shape().dim(1);

  if (weight.shape().dim(0) != hidden_size) {
    return Status::InvalidArgument(
        "CpuRmsNorm weight size must match hidden size");
  }

  if (hidden_size == 0) {
    return Status::InvalidArgument("CpuRmsNorm hidden size must be non-zero");
  }

  // row 表示当前归一化的一行。
  for (std::size_t row = 0; row < rows; ++row) {
    // row_offset 是当前行在连续内存里的起始下标。
    const std::size_t row_offset = row * hidden_size;

    // mean_square 累加平方均值，用于计算 RMS。
    float mean_square = 0.0f;

    for (std::size_t col = 0; col < hidden_size; ++col) {
      const float value = ReadFloat(input, row_offset + col);
      mean_square += value * value;
    }

    mean_square /= static_cast<float>(hidden_size);

    // scale 是 1 / sqrt(mean_square + epsilon)，epsilon 防止除零。
    const float scale = 1.0f / std::sqrt(mean_square + epsilon);

    for (std::size_t col = 0; col < hidden_size; ++col) {
      // value 是输入激活，weight_value 是当前 hidden 维度的缩放权重。
      const float value = ReadFloat(input, row_offset + col);
      const float weight_value = ReadFloat(weight, col);
      // output_value 是归一化并乘权重后的结果。
      const float output_value = value * scale * weight_value;

      WriteFloat(output, row_offset + col, output_value);
    }
  }

  return Status::Ok();
}

}  // namespace firstllm
