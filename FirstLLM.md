# FirstLLM 项目设计文档

## 1. 项目定位

FirstLLM 是一个基于 C++ 的极简大模型推理引擎实验项目。它的第一目标不是立刻追求极致性能，而是建立一套清晰、可扩展、可验证的推理引擎骨架，让后续可以逐步接入 CPU、CUDA、ROCm、NPU、WebGPU 等 backend，并继续扩展模型加载、KV cache、调度、量化、LoRA、服务化等能力。

一句话目标：

> 先做一个小而稳的 C++ 推理运行时，再把模型、算子、backend、调度器和服务层一层层长出来。

## 2. 参考架构与可借鉴点

本项目参考了多个成熟推理系统的公开设计，但会刻意保持实现规模更小。

### 2.1 ONNX Runtime: Execution Provider 思路

ONNX Runtime 通过 Execution Provider 抽象不同硬件加速库。它会询问 provider 能处理哪些节点或子图，并按优先级选择 provider；如果 CUDA 不支持某个算子，可以回退到 CPU。

FirstLLM 借鉴点：

- backend 不直接散落在 runtime 中，而是通过统一接口注册。
- runtime 可以按优先级选择 backend。
- backend 需要声明自己支持哪些算子、数据类型和设备。
- CPU backend 永远作为最小可用 fallback。

参考资料：

- https://onnxruntime.ai/docs/execution-providers/

### 2.2 TensorRT-LLM: Executor、Scheduler、KVCacheManager、Sampler 分层

TensorRT-LLM 的高层入口负责模型加载、tokenization、detokenization 和 generate API；内部 executor 通过后台循环处理请求，并把调度、KV cache、模型执行、采样拆成独立组件。

FirstLLM 借鉴点：

- 推理流程不要写成一个巨大的函数。
- 后续要把 `Scheduler`、`KVCacheManager`、`ModelExecutor`、`Sampler` 独立出来。
- 先做同步单请求，接口上预留批处理和异步生成空间。

参考资料：

- https://nvidia.github.io/TensorRT-LLM/architecture/overview.html

### 2.3 vLLM: 请求调度与 KV cache 是核心资产

vLLM 的架构把 API server、engine core、GPU worker 分开。engine core 负责 scheduler、KV cache 管理和模型执行协调；GPU worker 负责权重加载、forward 和设备内存管理。它也强调统一配置对象，方便快速增加新功能。

FirstLLM 借鉴点：

- 第一版可以单进程，但概念上保留 `Engine -> Scheduler -> Worker/Backend` 的路径。
- KV cache 从一开始就要作为独立模块设计，避免和 attention kernel 绑死。
- 配置集中在 `EngineConfig`，不要让后续每加一个功能就改一串构造函数。

参考资料：

- https://docs.vllm.ai/en/latest/design/arch_overview/
- https://docs.vllm.ai/en/latest/design/paged_attention/

### 2.4 llama.cpp: 本地、低依赖、C/C++、GGUF

llama.cpp 的目标是用较低门槛在多种硬件上本地运行大模型，使用 C/C++ 实现，支持 GGUF 模型生态，并拥有大量 examples、tools、tests。

FirstLLM 借鉴点：

- 起步阶段尽量少依赖，先保证 CMake 能构建、代码能跑。
- 目录结构要给 examples、tests、tools 留位置。
- 模型格式优先考虑 GGUF，因为它非常适合本地推理引擎学习和实验。

参考资料：

- https://github.com/ggml-org/llama.cpp

## 3. 第一阶段范围

第一阶段不要急着完整跑通 Llama，而是先完成以下能力：

- CMake 项目可构建。
- 提供 `firstllm` 静态库。
- 提供统一 backend 抽象。
- 提供 CPU backend。
- 提供最小 runtime `Engine`。
- 提供最小 `Tensor`、`Status`、`Device` 等基础类型。
- 提供 example 和 smoke test。
- `FirstLLM.md` 持续记录设计决策、路线图和问题。

第一阶段暂不做：

- 真正加载大模型权重。
- 真正实现 Transformer forward。
- 在线服务 API。
- CUDA kernel。
- 分布式推理。

这样做的原因很现实：大模型推理引擎最容易失败的地方不是某一个 kernel，而是项目边界混乱。先把接口与目录立好，后面的每一步才有地方放。

## 4. 推荐架构

```text
Application / CLI / Server
        |
        v
FirstLLM Runtime
  - Engine
  - Scheduler              later
  - KVCacheManager         later
  - Sampler                later
        |
        v
Model Layer
  - ModelConfig            later
  - ModelLoader            later
  - Graph / Module         later
        |
        v
Backend Abstraction
  - BackendRegistry
  - Backend interface
  - Device / capability
        |
        v
Backend Implementations
  - CPU backend
  - CUDA backend           later
  - ROCm backend           later
  - NPU backend            later
        |
        v
Kernels / Memory / Runtime APIs
```

### 4.1 Backend 接口职责

backend 应该回答三个问题：

- 我是谁：`BackendInfo`，包含名称、设备类型、优先级。
- 我能做什么：`Supports(OpKind, DType)`。
- 我是否可用：`Initialize()` 与 `IsAvailable()`。

后续可以继续扩展：

- `Allocate()` / `Free()`：设备内存管理。
- `Copy()`：Host 与 Device 数据搬运。
- `Execute()`：执行单个 op 或 graph partition。
- `Compile()`：把子图编译成 backend 内部可执行对象。

### 4.2 Runtime 职责

`Engine` 是用户侧入口。它不应该知道 CUDA kernel 的细节，也不应该知道 GGUF 文件的细节。它应该负责编排：

- 注册 backend。
- 初始化 backend。
- 读取配置。
- 接收请求。
- 选择调度路径。
- 返回生成结果。

### 4.3 CPU backend 的定位

CPU backend 是项目的地基。哪怕性能很弱，也要保证它：

- 永远可构建。
- 不依赖第三方库。
- 能作为所有算子的正确性参考。
- 在后续 CUDA/NPU backend 出问题时可以做对照测试。

## 5. 初始目录结构

当前建议搭建如下结构：

```text
FirstLLM/
  CMakeLists.txt
  FirstLLM.md
  include/
    firstllm/
      firstllm.h
      backends/
        cpu_backend.h
      core/
        backend.h
        status.h
        tensor.h
      runtime/
        engine.h
  src/
    backends/
      cpu/
        cpu_backend.cpp
    core/
      backend.cpp
      status.cpp
      tensor.cpp
    runtime/
      engine.cpp
  examples/
    firstllm_info.cpp
  tests/
    smoke.cpp
```

### 5.1 如何构建

安装 CMake 和任意支持 C++20 的编译器后，可以在项目根目录执行：

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

示例程序构建完成后通常位于：

```text
build/examples/firstllm_info
```

在 Windows + Visual Studio 生成器下，实际路径可能包含 `Debug` 或 `Release` 子目录。

## 6. 路线图

### M0: 工程骨架

- 完成 CMake。
- 完成 `Status`、`Tensor`、`Backend`、`Engine`。
- 完成 CPU backend 注册和初始化。
- 完成 example 和 smoke test。

### M1: 基础张量与 CPU 算子

- 支持 float32 tensor。
- 实现 `add`、`matmul`、`softmax`、`rms_norm`。
- 为每个算子写 CPU 正确性测试。

### M2: 模型格式与配置

- 增加 `ModelConfig`。
- 增加最小 GGUF metadata reader。
- 暂时只读模型结构和 tensor metadata，不急着完整加载全部权重。

### M3: Tiny Transformer

- 先支持一个极小 Llama-like block。
- 支持 embedding、rope、attention、mlp、lm_head。
- 用小尺寸随机权重跑通 forward。

### M4: 自回归生成

- 增加 tokenizer 接口。
- 增加 KV cache。
- 增加 greedy sampling、top-k、top-p、temperature。
- 跑通单请求文本生成。

### M5: CUDA backend

- backend 接口增加设备内存、copy、kernel dispatch。
- 先实现少量核心算子。
- 每个 CUDA 算子都要和 CPU backend 做数值对照。

### M6: 服务化与高级功能

- 异步请求队列。
- batch scheduler。
- streaming output。
- OpenAI-compatible API。
- LoRA。
- 量化。
- speculative decoding。

## 7. 当前设计决策

- 语言：C++20。
- 构建：CMake。
- 第三方依赖：第一阶段不引入。
- 初始 backend：CPU。
- backend 选择策略：名称匹配 + 优先级排序。
- 模型目标：优先 Llama-like decoder-only 模型。
- 模型格式倾向：GGUF，但先把 loader 接口留出来。

## 8. 已确认的项目方向

以下方向已经确认，会作为 M1 之后的路线依据：

1. 第一批支持 decoder-only 文本模型，优先面向 Llama/Qwen/Mistral 这类自回归生成模型；后续预留多模态扩展空间。
2. 第一块硬件 backend 优先做 CPU，同时 CUDA 要同步设计接口和最小实现路径。
3. 项目定位偏学习型代码，代码应重视清晰性、注释、阶段性文档和可验证的小步骤。
4. 模型格式直接兼容 GGUF，不先发明自定义权重格式。

对应的详细项目节点、章节进度和文件职责见 `ProjectNodes.md`。

## 9. 编码原则

- 所有模块先有接口，再有实现。
- CPU backend 是 correctness baseline。
- backend 之间不要互相依赖。
- runtime 不直接包含硬件细节。
- 每加一个算子，至少有一个 CPU 测试。
- 每加一个 backend，必须先声明 capability。
- 项目每进入一个 milestone，都更新本文件。
