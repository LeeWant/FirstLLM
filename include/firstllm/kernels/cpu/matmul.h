#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

// CPU 二维矩阵乘法：output[m, n] = lhs[m, k] * rhs[k, n]。
// 当前限制：只支持 2D float32、row-major 连续内存，不支持 batch、转置或优化 kernel。
Status CpuMatMul(const Tensor& lhs, const Tensor& rhs, Tensor* output);

}  // namespace firstllm
