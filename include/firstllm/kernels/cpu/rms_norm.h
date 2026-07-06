#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

Status CpuRmsNorm(const Tensor& input,
                  const Tensor& weight,
                  float epsilon,
                  Tensor* output);

}  // namespace firstllm