#include "firstllm/kernels/cpu/softmax.h"

#include <cmath>
#include <cstring>
#include <limits>

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

// 沿最后一维做 softmax，常用于 logits -> 概率分布。
Status CpuSoftmaxLastDim(const Tensor& input, Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuSoftmaxLastDim output tensor is null");
  }

  // 当前只支持 float32；mask、temperature、half 等能力后续再加。
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

  // last_dim 是每一行 softmax 的长度。
  const std::size_t last_dim = input.shape().dim(input.shape().rank() - 1);
  if (last_dim == 0) {
    return Status::InvalidArgument(
        "CpuSoftmaxLastDim last dimension must be non-zero");
  }

  // row_count 是除最后一维外的“行”数量。
  const std::size_t row_count = input.num_elements() / last_dim;

  // row 表示当前处理的 softmax 分组。
  for (std::size_t row = 0; row < row_count; ++row) {
    // row_offset 是当前分组在一维内存中的起点。
    const std::size_t row_offset = row * last_dim;

    // max_value 用于 max-subtraction，避免 exp(大数) 溢出。
    float max_value = -std::numeric_limits<float>::infinity();

    for (std::size_t col = 0; col < last_dim; ++col) {
      const float value = ReadFloat(input, row_offset + col);
      if (value > max_value) {
        max_value = value;
      }
    }

    // sum 是 exp(shifted logits) 的总和，用于归一化。
    float sum = 0.0f;

    for (std::size_t col = 0; col < last_dim; ++col) {
      // shifted 是减去最大值后的 logits。
      const float shifted = ReadFloat(input, row_offset + col) - max_value;
      // exp_value 先暂存在 output 中，避免额外分配临时数组。
      const float exp_value = std::exp(shifted);
      WriteFloat(output, row_offset + col, exp_value);
      sum += exp_value;
    }

    for (std::size_t col = 0; col < last_dim; ++col) {
      // value 是最终概率。
      const float value = ReadFloat(*output, row_offset + col) / sum;
      WriteFloat(output, row_offset + col, value);
    }
  }

  return Status::Ok();
}

}  // namespace firstllm
