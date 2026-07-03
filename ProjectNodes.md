# FirstLLM 从零手敲学习路线

## 1. 本文件的作用

这个文件是你从 0 到 1 手动实现 FirstLLM 的路线图。它不要求你一次理解整个推理引擎，而是把项目拆成很多很小的文件节点。你可以按照顺序逐个文件手敲、编译、运行、思考。

当前策略：

- 你手动创建每一个代码文件。
- 每个节点只引入一个或少数几个概念。
- 每个文件都要知道“它负责什么、为什么存在、暂时不负责什么”。
- 每完成一个节点，就更新 `ProgressLog.md`。
- CPU 路径先走通，CUDA 先保留设计位置，不急着实现。

保留的辅助文件：

- `FirstLLM.md`：项目总设计。
- `ProjectNodes.md`：你正在看的手敲路线。
- `ProgressLog.md`：实际进度日志。
- `agent.md`：协作和工作规则。
- `EnvironmentSetup.md`：环境搭建指南。
- `scripts/check-env.ps1`：环境检查脚本。
- `scripts/firstllm-env.ps1`：加载开发环境脚本。

已清理的代码/构建内容：

- `CMakeLists.txt`
- `include/`
- `src/`
- `examples/`
- `tests/`
- `build/`

## 2. 学习方式

每创建一个文件前，先回答三个问题：

1. 这个文件属于哪一层？
2. 它要解决什么问题？
3. 它暂时不应该解决什么问题？

每完成一个节点后，做三件事：

1. 编译或运行能验证的最小命令。
2. 把结果写入 `ProgressLog.md`。
3. 如果理解上有卡点，把卡点也写入 `ProgressLog.md`。

## 3. 总体路线

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

不要跳章。推理引擎不是只由某一个“核心代码”组成，而是由很多边界清楚的小模块拼起来。

## 4. 第 0 章：重新建立最小工程

### 0.1 创建 `CMakeLists.txt`

文件路径：

```text
CMakeLists.txt
```

所属层：

```text
Build System
```

作用：

- 声明 FirstLLM 是一个 C++20 项目。
- 告诉 CMake 哪些源码会组成 `firstllm` 库。
- 告诉 CMake 是否构建 examples 和 tests。

为什么需要：

- 没有构建系统，C++ 文件只是散落的文本。
- CMake 是后续添加 CPU、CUDA、测试、示例的统一入口。

暂时不做：

- 不引入第三方依赖。
- 不处理 CUDA。
- 不做复杂安装规则。

建议先写到只包含：

- `cmake_minimum_required`
- `project`
- `option(FIRSTLLM_BUILD_EXAMPLES ...)`
- `option(FIRSTLLM_BUILD_TESTS ...)`

这一小步写完后，不要求立刻 configure 成功，因为还没有源码。

### 0.2 创建空目录结构

手动创建：

```text
include/
include/firstllm/
include/firstllm/core/
include/firstllm/runtime/
include/firstllm/backends/
src/
src/core/
src/runtime/
src/backends/
src/backends/cpu/
examples/
tests/
```

作用：

- `include/` 放 public header。
- `src/` 放实现。
- `examples/` 放学习者可运行示例。
- `tests/` 放验证代码。

设计理念：

- 从第一天就让接口和实现分离。
- 目录结构本身就是架构图。

验证：

```powershell
rg --files
```

空目录不会被 `rg --files` 显示，这是正常现象。你也可以用文件管理器确认目录存在。

## 5. 第 1 章：Status 错误处理

### 1.1 创建 `include/firstllm/core/status.h`

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
- 如果只返回 `bool`，调用者不知道为什么失败。
- 如果到处抛异常，底层接口会更难统一。

核心概念：

```text
Status = ErrorCode + message
```

暂时不做：

- 不做复杂错误栈。
- 不做日志系统。
- 不做异常封装。

### 1.2 创建 `src/core/status.cpp`

作用：

- 实现 `Status::Ok()`。
- 实现 `Status::InvalidArgument()`。
- 实现 `Status::NotFound()`。
- 实现 `Status::BackendUnavailable()`。
- 实现 `Status::Internal()`。

学习重点：

- 头文件声明接口，cpp 文件实现行为。
- `Status` 是后续所有模块的共同语言。

### 1.3 更新 `CMakeLists.txt`

加入：

```text
src/core/status.cpp
```

此时可以先尝试 configure，但还不一定有完整库目标。

## 6. 第 2 章：Tensor 基础类型

### 2.1 创建 `include/firstllm/core/tensor.h`

作用：

- 定义 `DType`。
- 定义 `TensorShape`。
- 定义最小 `Tensor`。

为什么需要：

- 大模型推理中，输入 token、权重、激活值、logits 最终都会变成 tensor。
- tensor 是推理引擎的数据流单位。

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

### 2.2 创建 `src/core/tensor.cpp`

作用：

- 实现 dtype 到 byte size 的转换。
- 实现 shape 元素数量计算。
- 实现 tensor 内存分配。

学习重点：

- shape 和 memory size 的关系。
- 为什么 dtype 是 tensor 的一部分。
- 为什么第一版 tensor 越简单越好。

## 7. 第 3 章：Backend 抽象

### 3.1 创建 `include/firstllm/core/backend.h`

作用：

- 定义 `DeviceType`。
- 定义 `Device`。
- 定义 `OpKind`。
- 定义 `BackendInfo`。
- 定义 `Backend` 抽象类。
- 定义 `BackendRegistry`。

为什么需要：

- FirstLLM 未来要支持 CPU、CUDA、NPU 等 backend。
- runtime 不应该直接写死“调用 CPU 函数”或“调用 CUDA 函数”。
- backend 需要声明自己能做什么。

核心理念：

```text
Backend 不只是设备，它还是 capability provider。
```

暂时不做：

- 不实现真正算子执行。
- 不实现 graph partition。
- 不实现设备内存池。

### 3.2 创建 `src/core/backend.cpp`

作用：

- 实现 backend 注册。
- 实现按名称查找 backend。
- 实现按 op 和 dtype 选择 backend。

学习重点：

- registry 是插件式架构的最小形态。
- backend priority 是后续 CPU fallback 和 CUDA 优先的基础。

## 8. 第 4 章：CPU Backend

### 4.1 创建 `include/firstllm/backends/cpu_backend.h`

作用：

- 声明 `CpuBackend`。
- 继承 `Backend`。
- 作为 FirstLLM 的第一个真实 backend。

为什么 CPU 先做：

- CPU 最容易调试。
- CPU 不依赖额外硬件。
- CPU 是 CUDA 后续实现的正确性对照。

### 4.2 创建 `src/backends/cpu/cpu_backend.cpp`

作用：

- 返回 backend 名称 `cpu`。
- 返回设备类型 `DeviceType::kCPU`。
- 实现初始化。
- 声明第一批支持的 op。

暂时不做：

- 不真正执行 op。
- 不做多线程。
- 不做 SIMD。

## 9. 第 5 章：Engine Runtime

### 5.1 创建 `include/firstllm/runtime/engine.h`

作用：

- 定义 `EngineConfig`。
- 定义最小 `GenerationRequest`。
- 定义最小 `GenerationResult`。
- 定义 `Engine`。

为什么需要：

- 用户不应该直接面对 backend registry。
- `Engine` 是推理引擎对外入口。

设计边界：

- `Engine` 负责组织流程。
- `Engine` 不负责写 kernel。
- `Engine` 不负责解析 GGUF。

### 5.2 创建 `src/runtime/engine.cpp`

作用：

- 注册 backend。
- 初始化 backend。
- 暂时返回占位生成结果。

暂时不做：

- 不做 tokenizer。
- 不做模型 forward。
- 不做 KV cache。
- 不做 sampler。

## 10. 第 6 章：总入口、示例和测试

### 6.1 创建 `include/firstllm/firstllm.h`

作用：

- 聚合常用 public headers。
- 让 example 可以只 include 一个入口。

为什么需要：

- 学习者从 examples 进入项目时，入口应该简单。

### 6.2 创建 `examples/firstllm_info.cpp`

作用：

- 创建 `Engine`。
- 注册 `CpuBackend`。
- 初始化 runtime。
- 打印 backend 信息。
- 调用占位 `Generate()`。

为什么需要：

- example 是最好的“如何使用这个库”的说明。

### 6.3 创建 `tests/smoke.cpp`

作用：

- 验证最小路径能运行。
- 验证 CPU backend 注册成功。
- 验证 `Tensor` 的基础行为。

为什么需要：

- smoke test 不证明系统完整正确，但可以快速发现项目是否已经断掉。

### 6.4 更新 `CMakeLists.txt`

加入：

- `add_library(firstllm STATIC ...)`
- `target_compile_features(firstllm PUBLIC cxx_std_20)`
- `target_include_directories(firstllm PUBLIC include)`
- `add_executable(firstllm_info ...)`
- `add_executable(firstllm_smoke ...)`
- `enable_testing()`
- `add_test(...)`

验证命令：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; cmake -S . -B build -G 'Visual Studio 17 2022' -A x64"
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; cmake --build build --config Debug"
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; ctest --test-dir build -C Debug --output-on-failure"
.\build\Debug\firstllm_info.exe
```

完成标准：

- build 成功。
- smoke test 通过。
- example 输出 CPU backend。

## 11. 第 7 章：CPU add 算子

这一章开始 FirstLLM 才真正进入“计算”。

建议新增：

```text
include/firstllm/kernels/cpu/ops.h
src/kernels/cpu/add.cpp
tests/cpu_add_test.cpp
```

目标：

- 实现两个 float32 tensor 的逐元素相加。
- 检查 dtype。
- 检查 shape。
- 返回 `Status`。

学习重点：

- kernel 接口应该如何设计。
- shape mismatch 如何报告错误。
- CPU kernel 如何作为后续 CUDA kernel 的对照。

## 12. 第 8 章：CPU matmul 算子

建议新增：

```text
src/kernels/cpu/matmul.cpp
tests/cpu_matmul_test.cpp
```

目标：

- 实现最朴素的二维矩阵乘法。
- 不做优化，只追求清晰正确。

为什么重要：

- Transformer 的 linear projection 本质上就是 matmul。
- 后续 q/k/v projection、MLP、lm head 都会依赖它。

## 13. 第 9 章：softmax 与 rms_norm

建议新增：

```text
src/kernels/cpu/softmax.cpp
src/kernels/cpu/rms_norm.cpp
tests/cpu_softmax_test.cpp
tests/cpu_rms_norm_test.cpp
```

目标：

- softmax 使用 max-subtraction 保证数值稳定。
- rms_norm 理解 Llama-like 模型的归一化方式。

学习重点：

- 数值稳定性。
- epsilon。
- attention 和 transformer block 的基础组件。

## 14. 第 10 章：CUDA backend 骨架

先只做骨架，不写复杂 CUDA kernel。

建议新增：

```text
cmake/FirstLLMCuda.cmake
include/firstllm/backends/cuda_backend.h
src/backends/cuda/cuda_backend.cpp
tests/cuda_backend_test.cpp
```

目标：

- CUDA 是可选功能。
- 没有 CUDA 时 CPU 构建不受影响。
- CUDA backend 能声明是否可用。

暂时不做：

- 不急着实现 CUDA matmul。
- 不强行安装 CUDA。
- 不破坏 CPU-only 构建。

## 15. 第 11 章：GGUF metadata reader

建议新增：

```text
include/firstllm/model/gguf_reader.h
src/model/gguf_reader.cpp
tools/gguf_inspect.cpp
tests/gguf_reader_test.cpp
```

目标：

- 读取 GGUF magic。
- 读取 version。
- 读取 metadata。
- 读取 tensor info。

暂时不做：

- 不完整加载权重。
- 不 mmap。
- 不直接跑模型。

学习重点：

- 二进制文件格式。
- metadata 和 tensor data 的区别。
- 为什么 loader 不应该写在 `Engine` 中。

## 16. 后续章节概览

### Tiny Llama-like forward

开始把基础算子组合成模型结构：

- embedding。
- rms norm。
- q/k/v projection。
- rope。
- causal attention。
- MLP。
- residual。
- lm head。

### KV cache 与生成循环

理解 decoder-only 推理最重要的两段：

- prefill。
- decode。

### tokenizer 与 sampler

把文本生成闭环补齐：

- text -> token。
- logits -> next token。
- token -> text。

## 17. 每次学习记录模板

每完成一个文件或节点，在 `ProgressLog.md` 追加：

```text
时间：
阶段：
我手动创建/修改了哪些文件：
我理解了什么：
我还没理解什么：
运行了哪些验证命令：
结果：
下一步：
```

这份项目最重要的成果不只是代码，而是你能说清楚每一块为什么存在。

