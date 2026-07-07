#include "firstllm/kernels/cpu/matmul.h"

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

// 朴素三重循环矩阵乘法，优先保证索引清晰和正确性。
Status CpuMatMul(const Tensor& lhs, const Tensor& rhs, Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuMatMul output tensor is null");
  }

  // 当前只支持 float32；后续优化类型需要同步扩展测试。
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

  // m/k/n 对应矩阵乘法形状：(m x k) * (k x n) = (m x n)。
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

  // row/col 定位 output 的一个元素。
  for (std::size_t row = 0; row < m; ++row) {
    for (std::size_t col = 0; col < n; ++col) {
      // sum 累加 lhs[row, inner] * rhs[inner, col]。
      float sum = 0.0f;

      // inner 遍历左矩阵列和右矩阵行。
      for (std::size_t inner = 0; inner < k; ++inner) {
        const float lhs_value = ReadFloat(lhs, row * k + inner);
        const float rhs_value = ReadFloat(rhs, inner * n + col);
        sum += lhs_value * rhs_value;
      }

      // output 使用 row-major 线性索引 row * n + col。
      WriteFloat(output, row * n + col, sum);
    }
  }

  return Status::Ok();
}

}  // namespace firstllm
