# FirstLLM 项目设计文档

## 1. 项目定位

FirstLLM 是一个基于 C++20 的极简大模型推理引擎学习项目。它的第一目标不是立刻跑通真实大模型，也不是追求极致性能，而是用小步、可验证、可解释的方式建立一个推理引擎的骨架。

一句话目标：

> 先做一个小而稳的 C++ 推理运行时，再把 tensor、backend、kernel、runtime、model loader、KV cache、sampler 和服务层一层层长出来。

当前项目采用“从零手敲学习模式”：

- 用户亲手创建代码文件、测试文件并运行验证。
- Agent 负责解释、检查、维护项目文档和进度日志。
- 每个模块都先讲清楚职责和边界，再进入实现。

## 2. 当前阶段

当前阶段处于早期工程骨架和核心基础类型阶段。

已完成：

- CMake 最小工程。
- 基础目录结构。
- `Status` 错误处理模块。
- `Status` 最小测试。
- `Tensor` 基础类型。
- `Tensor` 最小测试。
- `Backend` 抽象。
- `BackendRegistry` 最小测试。
- `CPU Backend` 能力声明。
- `CPU Backend` 最小测试。
- `Engine Runtime` 最小入口。
- `Engine Runtime` 最小测试。
- `firstllm` 静态库目标。
- CTest 测试闭环。

下一步：

- 创建总入口 header、示例程序和 smoke test。
- 提供一个能从外部包含并验证基础 runtime 路径的最小入口。

当前关键文件：

```text
CMakeLists.txt
include/firstllm/core/status.h
include/firstllm/core/tensor.h
include/firstllm/core/backend.h
include/firstllm/backends/cpu_backend.h
include/firstllm/runtime/engine.h
src/core/status.cpp
src/core/tensor.cpp
src/core/backend.cpp
src/backends/cpu/cpu_backend.cpp
src/runtime/engine.cpp
tests/status_test.cpp
tests/tensor_test.cpp
tests/backend_test.cpp
tests/cpu_backend_test.cpp
tests/engine_test.cpp
```

## 3. 参考架构与借鉴点

FirstLLM 参考成熟推理系统的公开设计，但会保持实现规模更小。

### 3.1 ONNX Runtime: Execution Provider 思路

借鉴点：

- backend 通过统一接口注册。
- runtime 可以按能力和优先级选择 backend。
- CPU backend 作为最小可用 fallback。

### 3.2 TensorRT-LLM: 分层执行

借鉴点：

- 推理流程不要写成一个巨大函数。
- 后续将 `Scheduler`、`KVCacheManager`、`ModelExecutor`、`Sampler` 拆成独立组件。
- 先做同步单请求，再考虑异步和 batch。

### 3.3 vLLM: KV cache 与调度是核心

借鉴点：

- 概念上保留 `Engine -> Scheduler -> Worker/Backend` 的路径。
- KV cache 后续作为独立模块设计。
- 配置集中到 `EngineConfig`，避免构造函数不断膨胀。

### 3.4 llama.cpp: 本地、低依赖、GGUF

借鉴点：

- 起步阶段尽量少依赖。
- 保留 examples、tests、tools 的工程位置。
- 长期兼容 GGUF 模型生态。

## 4. 第一阶段范围

第一阶段目标：

- CMake 项目可构建。
- 提供 `firstllm` 静态库。
- 提供 `Status`、`Tensor`、`Device`、`Backend` 等核心类型。
- 提供 CPU backend。
- 提供最小 runtime `Engine`。
- 提供 example 和 smoke test。
- 每个核心节点都有最小验证。

第一阶段暂不做：

- 不加载真实大模型权重。
- 不实现完整 Transformer forward。
- 不做在线服务 API。
- 不写 CUDA kernel。
- 不做分布式推理。

原因很简单：推理引擎最容易混乱的地方不是某个 kernel，而是模块边界。先把边界立稳，后面的每个功能才有地方放。

## 5. 推荐架构

```text
Application / Examples / Tools
        |
        v
Runtime
  - Engine
  - Scheduler              later
  - KVCacheManager         later
  - Sampler                later
        |
        v
Model Layer
  - ModelConfig            later
  - GGUF Reader            later
  - ModelLoader            later
  - Llama-like Model       later
        |
        v
Backend Abstraction
  - BackendRegistry
  - Backend interface
  - Device / Capability
        |
        v
Backend Implementations
  - CPU backend
  - CUDA backend           later
        |
        v
Kernels / Memory / Device APIs
```

### 5.1 Core 层

Core 层提供所有模块都会用到的基础语言：

- `Status`：表达成功或失败。
- `Tensor`：表达数据、shape、dtype 和内存。
- `Device`：表达计算设备。
- `Backend`：表达硬件能力提供者。

### 5.2 Backend 层

backend 应回答三个问题：

- 我是谁：名称、设备类型、优先级。
- 我能做什么：支持哪些 op 和 dtype。
- 我是否可用：初始化是否成功、当前设备是否可用。

### 5.3 Runtime 层

`Engine` 是用户侧入口。它负责编排，不应直接知道 CUDA kernel 细节，也不应直接解析 GGUF 文件。

### 5.4 CPU backend 定位

CPU backend 是 correctness baseline。即使性能一般，也必须：

- 永远可构建。
- 尽量不依赖第三方库。
- 能作为 CUDA/NPU backend 的对照。

## 6. 目录结构

当前和近期目标目录：

```text
FirstLLM/
  CMakeLists.txt
  FirstLLM.md
  ProjectNodes.md
  ProgressLog.md
  agent.md
  EnvironmentSetup.md
  include/
    firstllm/
      core/
        status.h
        tensor.h
        backend.h
      backends/
        cpu_backend.h
    runtime/
      engine.h
  src/
    core/
      status.cpp
      tensor.cpp
      backend.cpp
    backends/
      cpu/
        cpu_backend.cpp
    runtime/
      engine.cpp
  examples/
  tests/
    status_test.cpp
    tensor_test.cpp
    backend_test.cpp
    cpu_backend_test.cpp
    engine_test.cpp
```

## 7. 构建与测试

推荐在项目根目录执行：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; cmake -S . -B build -G 'Visual Studio 17 2022' -A x64"
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; cmake --build build --config Debug"
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; ctest --test-dir build -C Debug --output-on-failure"
```

当前已验证：

```text
CTest: 100% tests passed, 0 tests failed out of 5
```

## 8. 路线图

### M0: 工程骨架

- CMake。
- 基础目录。
- `Status`。
- 最小测试闭环。

当前状态：已完成 `Status`、`Tensor`、`Backend` 抽象、`CPU Backend` 和 `Engine Runtime`，下一步是总入口、示例程序和 smoke test。

### M1: 基础张量与 CPU 算子

- `Tensor`。
- `add`。
- `matmul`。
- `softmax`。
- `rms_norm`。

### M2: Backend 与 Runtime

- `Backend` 抽象。
- CPU backend。
- `Engine` 最小入口。
- example 和 smoke test。

### M3: 模型格式与配置

- `ModelConfig`。
- GGUF metadata reader。
- tensor metadata。

### M4: Tiny Transformer

- embedding。
- rope。
- attention。
- MLP。
- lm head。

### M5: 自回归生成

- tokenizer。
- KV cache。
- sampler。
- 文本生成闭环。

### M6: CUDA 与高级能力

- CUDA backend。
- CUDA kernel 对照测试。
- streaming。
- OpenAI-compatible API。
- LoRA。
- 量化。

## 9. 当前设计决策

- 语言：C++20。
- 构建：CMake。
- 第三方依赖：早期不引入。
- 初始 backend：CPU。
- 测试策略：先使用标准库 `assert` 和 CTest。
- 学习策略：用户手敲代码，Agent 维护文档与日志。
- 模型方向：Llama-like decoder-only。
- 模型格式：长期倾向 GGUF。

## 10. 编码原则

- 所有模块先有接口，再有实现。
- 每个核心模块都要有最小测试。
- CPU 是正确性基线。
- backend 之间不要互相依赖。
- runtime 不直接包含硬件细节。
- model loader 不写进 `Engine`。
- 不过早优化，不提前引入复杂抽象。
- 每完成一个节点，由 Agent 更新 `ProgressLog.md`。
