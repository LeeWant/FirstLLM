#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

namespace firstllm {

// CPU softmax：沿最后一维把 logits 转成概率分布。
// 当前限制：只支持 float32，不支持 mask、temperature、in-place API 或非连续内存。
Status CpuSoftmaxLastDim(const Tensor& input, Tensor* output);

}  // namespace firstllm
