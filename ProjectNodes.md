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
第 5 章：Engine Runtime                下一步
```

当前已存在的重要文件：

```text
CMakeLists.txt
include/firstllm/core/status.h
include/firstllm/core/tensor.h
include/firstllm/core/backend.h
include/firstllm/backends/cpu_backend.h
src/core/status.cpp
src/core/tensor.cpp
src/core/backend.cpp
src/backends/cpu/cpu_backend.cpp
tests/status_test.cpp
tests/tensor_test.cpp
tests/backend_test.cpp
tests/cpu_backend_test.cpp
```

当前已验证：

```text
CMake configure 成功
CMake build 成功
CTest: 100% tests passed, 0 tests failed out of 4
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
第 11 章：GGUF metadata reader
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

状态：下一步。

目标文件：

```text
include/firstllm/runtime/engine.h
src/runtime/engine.cpp
```

作用：

- 定义 `EngineConfig`。
- 定义最小请求和结果结构。
- 定义用户侧入口 `Engine`。

边界：

- `Engine` 负责编排。
- `Engine` 不写 kernel。
- `Engine` 不解析 GGUF。

## 11. 第 6 章：总入口、示例和 smoke test

状态：未开始。

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

## 12. 后续章节概览

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

### 第 11 章：GGUF metadata reader

目标：

- 读取 GGUF magic、version、metadata、tensor info。
- 不急着完整加载权重。

### 第 12 到 14 章：Tiny forward、KV cache、tokenizer、sampler

目标：

- 逐步把基础算子组合成极小 decoder-only 推理闭环。

## 13. 每章完成标准

每章完成时至少满足：

- 代码文件存在。
- CMake 已接入相关源码或测试。
- 能运行最小验证命令。
- `ProgressLog.md` 已由 Agent 更新。
- 下一步在本文件中清楚可见。
