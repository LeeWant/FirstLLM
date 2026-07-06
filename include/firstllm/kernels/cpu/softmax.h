#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

Status CpuSoftmaxLastDim(const Tensor& input, Tensor* output);

}  // namespace firstllm