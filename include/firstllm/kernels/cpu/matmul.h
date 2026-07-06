#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

Status CpuMatMul(const Tensor& lhs, const Tensor& rhs, Tensor* output);

}  // namespace firstllm