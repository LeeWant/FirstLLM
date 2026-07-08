#pragma once

// FirstLLM 公共总入口：示例程序或外部使用者优先包含这个文件。
// 当前聚合核心类型、backend、Engine 和最小生成接口；kernel/model 头文件按需单独包含。

#include "firstllm/backends/cpu_backend.h"
#include "firstllm/core/backend.h"
#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"
#include "firstllm/runtime/engine.h"
#include "firstllm/runtime/generator.h"
