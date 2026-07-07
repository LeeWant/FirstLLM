#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

// CPU 逐元素加法：output[i] = lhs[i] + rhs[i]。
// 当前限制：只支持 float32、相同 shape、连续 host memory，不支持广播或 in-place API。
Status CpuAdd(const Tensor& lhs, const Tensor& rhs, Tensor* output);

}  // namespace firstllm
