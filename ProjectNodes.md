# FirstLLM 从零手敲学习路线

## 1. 文件作用

本文件是 FirstLLM 的章节路线图。它负责回答三个问题：

- 现在做到哪一章？
- 下一步应该创建哪些文件？
- 每个文件为什么存在、暂时不应该负责什么？

当前项目采用“用户手敲代码，Agent 维护路线与文档”的方式推进。用户负责创建代码、运行验证；Agent 负责解释、检查、维护 `FirstLLM.md`、`ProjectNodes.md`、`agent.md` 和 `ProgressLog.md`。

## 2. 当前状态

```text
第 0 章：重新建立最小工程              已完成
第 1 章：Status 错误处理               已完成
第 1 章补充：Status 测试               已完成
第 2 章：Tensor 基础类型               已完成
第 3 章：Backend 抽象                  已完成
第 4 章：CPU Backend                   已完成
第 5 章：Engine Runtime                已完成
第 6 章：示例程序和 smoke test         已完成
第 7 章：CPU add 算子                  已完成
第 8 章：CPU matmul 算子               已完成
第 9 章：softmax 与 rms_norm           已完成
第 10 章：CUDA backend 骨架            已完成
第 11 章：GGUF reader                  已完成首版
第 12 章：Tiny Llama-like forward      已完成首版
第 13 章：KV cache 与自回归生成        已完成首版
```

当前已存在的重要文件：

```text
CMakeLists.txt
include/firstllm/core/status.h
include/firstllm/core/tensor.h
include/firstllm/core/backend.h
include/firstllm/backends/cpu_backend.h
include/firstllm/backends/cuda_backend.h
include/firstllm/runtime/engine.h
include/firstllm/runtime/generator.h
include/firstllm/model/gguf_reader.h
include/firstllm/model/tiny_llama.h
include/firstllm/model/kv_cache.h
include/firstllm/firstllm.h
include/firstllm/kernels/cpu/add.h
include/firstllm/kernels/cpu/matmul.h
include/firstllm/kernels/cpu/softmax.h
include/firstllm/kernels/cpu/rms_norm.h
src/core/status.cpp
src/core/tensor.cpp
src/core/backend.cpp
src/backends/cpu/cpu_backend.cpp
src/backends/cuda/cuda_backend.cpp
src/runtime/engine.cpp
src/runtime/generator.cpp
src/model/gguf_reader.cpp
src/model/tiny_llama.cpp
src/model/kv_cache.cpp
src/kernels/cpu/add.cpp
src/kernels/cpu/matmul.cpp
src/kernels/cpu/softmax.cpp
src/kernels/cpu/rms_norm.cpp
examples/firstllm_info.cpp
tests/status_test.cpp
tests/tensor_test.cpp
tests/backend_test.cpp
tests/cpu_backend_test.cpp
tests/cuda_backend_test.cpp
tests/engine_test.cpp
tests/generator_test.cpp
tests/gguf_reader_test.cpp
tests/tiny_llama_test.cpp
tests/kv_cache_test.cpp
tests/smoke.cpp
tests/cpu_add_test.cpp
tests/cpu_matmul_test.cpp
tests/cpu_softmax_test.cpp
tests/cpu_rms_norm_test.cpp
```

当前已验证：

```text
CMake configure 成功
CMake build 成功
CTest: 100% tests passed, 0 tests failed out of 15
```

## 3. 标准学习流程

每个节点都按同一套流程推进：

1. 先回答：这个文件属于哪一层？
2. 再回答：它要解决什么问题？
3. 再回答：它暂时不解决什么问题？
4. 用户手动创建或修改文件。
5. 用户运行最小验证命令。
6. Agent 根据反馈检查结果。
7. Agent 更新 `ProgressLog.md`。

这个项目的成果不只是代码，而是你能说清楚每一块为什么存在。

## 4. 总体路线

```text
第 0 章：重新建立最小工程
第 1 章：Status 错误处理
第 2 章：Tensor 基础类型
第 3 章：Backend 抽象
第 4 章：CPU Backend
第 5 章：Engine Runtime
第 6 章：示例程序和 smoke test
第 7 章：CPU add 算子
第 8 章：CPU matmul 算子
第 9 章：softmax 与 rms_norm
第 10 章：CUDA backend 骨架
第 11 章：GGUF reader
第 12 章：Tiny Llama-like forward
第 13 章：KV cache 与自回归生成
第 14 章：tokenizer、sampler、真实文本生成
```

不要跳章。推理引擎不是一个巨大函数，而是一组边界清楚的小模块。

## 5. 第 0 章：重新建立最小工程

状态：已完成。

完成内容：

- 创建 `CMakeLists.txt`。
- 创建 `include/`、`src/`、`examples/`、`tests/` 等基础目录。
- 运行 CMake configure，确认最小工程可被 CMake 识别。

设计重点：

- `CMakeLists.txt` 第一版只声明项目和构建开关。
- 目录结构先行，让接口、实现、示例、测试从第一天就分开。

## 6. 第 1 章：Status 错误处理

状态：已完成。

### 1.1 `include/firstllm/core/status.h`

所属层：

```text
Core
```

作用：

- 定义 `ErrorCode`。
- 定义 `Status`。
- 用统一方式表达成功或失败。

为什么需要：

- 推理引擎底层会频繁遇到文件不存在、shape 不匹配、backend 不可用、模型格式错误等情况。
- `bool` 只能表达成败，不能表达失败原因。
- 统一的 `Status` 会成为后续模块共同使用的错误语言。

暂时不做：

- 不做复杂错误栈。
- 不做日志系统。
- 不做异常封装。

### 1.2 `src/core/status.cpp`

作用：

- 实现 `Status::Ok()`。
- 实现 `Status::InvalidArgument()`。
- 实现 `Status::NotFound()`。
- 实现 `Status::BackendUnavailable()`。
- 实现 `Status::Internal()`。
- 实现 `ok()`、`code()`、`message()`。

学习重点：

- 头文件声明接口，cpp 文件实现行为。
- `Status = ErrorCode + message`。

### 1.3 `tests/status_test.cpp`

状态：已完成。

作用：

- 验证 `Status::Ok()` 的成功状态。
- 验证 `Status::InvalidArgument()` 的错误码和错误信息。
- 建立第一个“代码 + 测试 + CMake + CTest”的闭环。

暂时不做：

- 不引入第三方测试框架。
- 不测试其它模块。
- 不追求完整覆盖所有错误码。

完成标准：

- `ctest` 输出 `100% tests passed`。

## 7. 第 2 章：Tensor 基础类型

状态：已完成。

本章目标是建立 FirstLLM 的最小数据容器。大模型中的 token、权重、激活值、logits 最终都会变成 tensor，所以这一章是后续算子的地基。

### 2.1 创建 `include/firstllm/core/tensor.h`

所属层：

```text
Core
```

作用：

- 定义 `DType`。
- 定义 `TensorShape`。
- 定义最小 `Tensor`。

第一版只支持：

- host memory。
- `float32`、`int32`、`uint8`。
- shape。
- raw bytes。

暂时不做：

- 不做 GPU memory。
- 不做复杂 stride。
- 不做 view。
- 不做量化 tensor。

完成情况：

- 用户已手动创建该文件。
- 已定义 `DType`、`TensorShape` 和最小 `Tensor` 接口。

### 2.2 创建 `src/core/tensor.cpp`

作用：

- 实现 dtype 到 byte size 的转换。
- 实现 shape 元素数量计算。
- 实现 tensor 内存分配。

学习重点：

- shape 和 memory size 的关系。
- dtype 为什么必须成为 tensor 的一部分。
- 第一版 tensor 越简单，后续越容易验证。

完成情况：

- 用户已手动创建该文件。
- 已实现 dtype byte size、shape 元素数量计算和 tensor host memory 分配。

### 2.3 创建 `tests/tensor_test.cpp`

作用：

- 验证 dtype byte size。
- 验证 shape element count。
- 验证 tensor 按 dtype 和 shape 分配正确字节数。

暂时不做：

- 不测试算子。
- 不测试 GPU。
- 不测试复杂内存布局。

完成情况：

- 用户已手动创建该文件。
- 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 2`。

## 8. 第 3 章：Backend 抽象

状态：已完成。

目标文件：

```text
include/firstllm/core/backend.h
src/core/backend.cpp
tests/backend_test.cpp
```

作用：

- 定义 `DeviceType`、`Device`、`OpKind`、`BackendInfo`。
- 定义 `Backend` 抽象类。
- 定义 `BackendRegistry`。

核心理念：

```text
Backend 不只是设备，它还是 capability provider。
```

完成情况：

- 用户已手动创建 `include/firstllm/core/backend.h`。
- 用户已手动创建 `src/core/backend.cpp`。
- 用户已手动创建 `tests/backend_test.cpp`。
- 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 3`。

## 9. 第 4 章：CPU Backend

状态：已完成。

目标文件：

```text
include/firstllm/backends/cpu_backend.h
src/backends/cpu/cpu_backend.cpp
tests/cpu_backend_test.cpp
```

作用：

- 声明并实现第一个真实 backend。
- CPU backend 作为 correctness baseline。
- 后续 CUDA backend 的结果应与 CPU 对照。

完成情况：

- 用户已手动创建 `include/firstllm/backends/cpu_backend.h`。
- 用户已手动创建 `src/backends/cpu/cpu_backend.cpp`。
- 用户已手动创建 `tests/cpu_backend_test.cpp`。
- 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 4`。

## 10. 第 5 章：Engine Runtime

状态：已完成。

目标文件：

```text
include/firstllm/runtime/engine.h
src/runtime/engine.cpp
tests/engine_test.cpp
```

作用：

- 定义 `EngineConfig`。
- 定义最小请求和结果结构。
- 定义用户侧入口 `Engine`。

边界：

- `Engine` 负责编排。
- `Engine` 不写 kernel。
- `Engine` 不解析 GGUF。

完成情况：

- 用户已手动创建 `include/firstllm/runtime/engine.h`。
- 用户已手动创建 `src/runtime/engine.cpp`。
- 用户已手动创建 `tests/engine_test.cpp`。
- 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 5`。

## 11. 第 6 章：总入口、示例和 smoke test

状态：已完成。

目标文件：

```text
include/firstllm/firstllm.h
examples/firstllm_info.cpp
tests/smoke.cpp
```

作用：

- 提供统一 public header。
- 提供可运行示例。
- 提供最小系统级 smoke test。

完成情况：

- 用户已手动创建 `include/firstllm/firstllm.h`。
- 用户已手动创建 `examples/firstllm_info.cpp`。
- 用户已手动创建 `tests/smoke.cpp`。
- 已接入 CMake、示例构建和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 6`。

## 12. 第 7 章：CPU add 算子

状态：已完成。

目标文件：

```text
include/firstllm/kernels/cpu/add.h
src/kernels/cpu/add.cpp
tests/cpu_add_test.cpp
```

作用：

- 实现 float32 tensor 逐元素相加。
- 检查输入输出 tensor 的 dtype。
- 检查输入输出 tensor 的 shape。
- 使用 `Status` 返回参数错误或成功状态。

完成情况：

- 用户已手动创建 `include/firstllm/kernels/cpu/add.h`。
- 用户已手动创建 `src/kernels/cpu/add.cpp`。
- 用户已手动创建 `tests/cpu_add_test.cpp`。
- 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 7`。

## 13. 第 8 章：CPU matmul 算子

状态：已完成。

目标文件：

```text
include/firstllm/kernels/cpu/matmul.h
src/kernels/cpu/matmul.cpp
tests/cpu_matmul_test.cpp
```

作用：

- 实现最朴素的二维 float32 矩阵乘法。
- 检查输入输出 tensor 的 dtype。
- 检查 lhs、rhs 和 output 的 shape 关系。
- 使用 `Status` 返回参数错误或成功状态。

完成情况：

- 用户已手动创建 `include/firstllm/kernels/cpu/matmul.h`。
- 用户已手动创建 `src/kernels/cpu/matmul.cpp`。
- 用户已手动创建 `tests/cpu_matmul_test.cpp`。
- 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 8`。

## 14. 第 9 章：softmax 与 rms_norm

状态：已完成。

目标文件：

```text
include/firstllm/kernels/cpu/softmax.h
src/kernels/cpu/softmax.cpp
tests/cpu_softmax_test.cpp
include/firstllm/kernels/cpu/rms_norm.h
src/kernels/cpu/rms_norm.cpp
tests/cpu_rms_norm_test.cpp
```

作用：

- 实现二维 `float32` tensor 的最后一维 softmax。
- softmax 使用 max-subtraction 保证数值稳定。
- 实现 Llama-like 模型常用的 RMSNorm。
- 继续使用 `Status` 表达参数错误。

完成情况：

- 用户已手动创建 `include/firstllm/kernels/cpu/softmax.h`。
- 用户已手动创建 `src/kernels/cpu/softmax.cpp`。
- 用户已手动创建 `tests/cpu_softmax_test.cpp`。
- softmax 已接入 CMake 和 CTest。
- 用户已手动创建 `include/firstllm/kernels/cpu/rms_norm.h`。
- 用户已手动创建 `src/kernels/cpu/rms_norm.cpp`。
- 用户已手动创建 `tests/cpu_rms_norm_test.cpp`。
- rms_norm 已接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 10`。

下一步：

- 进入第 10 章，创建 CUDA backend 骨架。

## 15. 第 10 章：CUDA backend 骨架

状态：已完成。

目标文件：

```text
include/firstllm/backends/cuda_backend.h
src/backends/cuda/cuda_backend.cpp
tests/cuda_backend_test.cpp
```

作用：

- 用和 CPU backend 相同的 `Backend` 抽象声明 CUDA backend。
- 让 CUDA 成为可选能力，而不是当前构建的硬依赖。
- 没有 CUDA toolkit 时仍然保持 CPU-only 构建和测试通过。
- 第一版只做 backend 信息、初始化状态和能力查询，不写 CUDA kernel。

完成情况：

- 用户已手动创建 `include/firstllm/backends/cuda_backend.h`。
- 用户已手动创建 `src/backends/cuda/cuda_backend.cpp`。
- 用户已手动创建 `tests/cuda_backend_test.cpp`。
- 用户已将 `src/backends/cuda/cuda_backend.cpp` 接入 `firstllm` 静态库目标。
- 用户已将 `firstllm_cuda_backend_test` 接入 CMake 和 CTest。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 11`。

下一步：

- 进入第 11 章，创建 GGUF reader。

## 16. 第 11 章：GGUF reader

状态：已完成首版。

目标文件：

```text
include/firstllm/model/gguf_reader.h
src/model/gguf_reader.cpp
tests/gguf_reader_test.cpp
```

作用：

- 读取 GGUF 文件头的 magic 和 version。
- 为后续读取 metadata、tensor info 和权重数据做入口。
- 第一版只验证格式识别和基本错误处理，不加载真实大模型权重。

完成情况：

- 用户已手动创建 `include/firstllm/model/gguf_reader.h`。
- 用户已手动创建 `src/model/gguf_reader.cpp`。
- 用户已手动创建 `tests/gguf_reader_test.cpp`。
- 用户已将 `src/model/gguf_reader.cpp` 接入 `firstllm` 静态库目标。
- 用户已将 `firstllm_gguf_reader_test` 接入 CMake 和 CTest。
- `GgufReader::read_header()` 已能读取 magic、version、tensor_count 和 metadata_kv_count。
- `GgufReader::read_metadata()` 已能读取 `uint32`、`uint64`、`bool` 和 `string` metadata value。
- `GgufReader::read_tensor_infos()` 已能读取 tensor 名称、维度、类型和 offset。
- `GgufReader::data_section_offset()` 已能返回对齐后的 data section 文件起点。
- `GgufTensorInfo::data_offset` 已能保存每个 tensor 数据在文件中的绝对偏移。
- 测试已覆盖正常 header、正常 metadata、正常 tensor info、默认对齐、自定义 `general.alignment`、空路径、文件不存在、magic 错误、截断文件、不支持的 metadata 类型、非法 bool、非法 alignment、非法 tensor 维度数量、截断 tensor info 和 tensor offset 溢出。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 12`。

下一步：

- 进入第 12 章，搭建 Tiny Llama-like forward 的最小可验证路径。

## 17. 第 12 章：Tiny Llama-like forward

状态：已完成首版。

目标文件：

```text
include/firstllm/model/tiny_llama.h
src/model/tiny_llama.cpp
tests/tiny_llama_test.cpp
```

作用：

- 在 model 层建立第一条可验证的 forward 路径。
- 串联已有 CPU kernel：RMSNorm、MatMul、Add 和 Softmax。
- 使用内存中的小权重和小输入验证模型层如何组织 kernel 调用。
- 暂不加载真实 GGUF 权重，不做 attention、MLP、RoPE、KV cache、tokenizer 或 sampler。

完成情况：

- Agent 接手创建 `include/firstllm/model/tiny_llama.h`。
- Agent 接手创建 `src/model/tiny_llama.cpp`。
- Agent 接手创建 `tests/tiny_llama_test.cpp`。
- 新增 `TinyLlamaConfig`、`TinyLlamaWeights` 和 `TinyLlamaModel`。
- `TinyLlamaModel::forward()` 已能执行 `hidden_states -> RMSNorm -> MatMul -> Add bias -> Softmax`。
- 测试已覆盖正常 forward、softmax 概率和、空 output、错误 dtype、错误 rank、错误 output shape、缺失权重、错误 bias shape 和坏配置。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 13`。

下一步：

- 进入第 13 章，设计 KV cache 与自回归生成的最小数据结构和单步接口。

## 18. 第 13 章：KV cache 与自回归生成

状态：已完成首版。

目标文件：

```text
include/firstllm/model/kv_cache.h
src/model/kv_cache.cpp
tests/kv_cache_test.cpp
include/firstllm/runtime/generator.h
src/runtime/generator.cpp
tests/generator_test.cpp
```

作用：

- 为自回归生成保存每一层历史 token 的 key/value。
- 支持按 layer 追加一个 token 的 KV，并能按 layer/token/head/dim 读取单个元素。
- 建立后续 attention 查询历史 KV 的状态基础。
- 建立 greedy 自回归单步生成接口：forward -> argmax -> 记录 token。
- 暂不做分页 KV cache、batch、GPU memory、高性能 attention、tokenizer、随机 sampler 或完整生成循环。

完成情况：

- Agent 接手创建 `include/firstllm/model/kv_cache.h`。
- Agent 接手创建 `src/model/kv_cache.cpp`。
- Agent 接手创建 `tests/kv_cache_test.cpp`。
- 新增 `KvCacheConfig` 和 `KvCache`。
- `KvCache::append()` 已能向指定 layer 追加一帧 `[kv_head_count, head_dim]` 的 key/value。
- `KvCache::read_key()` 和 `KvCache::read_value()` 已能读取缓存中的单个 float32 元素。
- `KvCache::clear()` 已能清空所有 layer 的 token 计数。
- 测试已覆盖正常追加、按层计数、读取 key/value、容量满、未写 token、layer/head/dim 越界、空输出指针、错误 dtype、错误 rank、错误 shape、clear 和无效配置。
- Agent 接手创建 `include/firstllm/runtime/generator.h`。
- Agent 接手创建 `src/runtime/generator.cpp`。
- Agent 接手创建 `tests/generator_test.cpp`。
- 新增 `GeneratorConfig` 和 `GenerationState`。
- `GreedyNextToken()` 已能从最后一行概率分布中选择最大概率 token。
- `GenerateOneStep()` 已能调用 `TinyLlamaModel::forward()`，用 greedy 得到 next token，并更新 `GenerationState`。
- 测试已覆盖 greedy 正常路径、tie 规则、空输出指针、错误 dtype/rank、单步生成状态推进、max token 限制、eos finished、空 state/next 指针、坏生成配置和错误 hidden_state。
- 当前 `ctest` 结果为 `100% tests passed, 0 tests failed out of 15`。

下一步：

- 进入第 14 章，准备 tokenizer、sampler 和真实文本生成的最小接口。

## 19. 后续章节概览

### 第 7 章：CPU add 算子

目标：

- 实现 float32 tensor 逐元素相加。
- 检查 dtype。
- 检查 shape。
- 返回 `Status`。

### 第 8 章：CPU matmul 算子

目标：

- 实现最朴素二维矩阵乘法。
- 不优化，只追求清晰正确。

### 第 9 章：softmax 与 rms_norm

目标：

- softmax 使用 max-subtraction 保证数值稳定。
- rms_norm 为 Llama-like 模型做准备。

### 第 10 章：CUDA backend 骨架

目标：

- CUDA 作为可选能力。
- 没有 CUDA 时 CPU-only 构建不受影响。

### 第 11 章：GGUF reader

目标：

- 读取 GGUF magic、version、metadata、tensor info，并为权重数据定位做准备。
- 不急着完整加载权重。

### 第 14 章：tokenizer、sampler、真实文本生成

目标：

- 在 Tiny forward、KV cache 和单步生成基础上逐步加入 tokenizer、sampler 和真实文本生成。

## 20. 每章完成标准

每章完成时至少满足：

- 代码文件存在。
- CMake 已接入相关源码或测试。
- 能运行最小验证命令。
- `ProgressLog.md` 已由 Agent 更新。
- 下一步在本文件中清楚可见。
