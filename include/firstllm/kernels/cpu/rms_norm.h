#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

// CPU RMSNorm：按行计算 root mean square，再乘以 weight。
// 当前限制：input 必须是 2D float32，weight 必须是 1D float32，暂不支持 half/bfloat16。
Status CpuRmsNorm(const Tensor& input,
                  const Tensor& weight,
                  float epsilon,
                  Tensor* output);

}  // namespace firstllm
