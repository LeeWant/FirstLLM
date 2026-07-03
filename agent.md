# FirstLLM Agent 工作指南

## 1. 文件目的

本文件是 FirstLLM 的协作协议。它说明后续智能体或协作者进入项目后应该如何阅读、如何指导、如何维护文档，以及哪些事情不应该越界代劳。

当前项目是一个学习型 C++ 推理引擎项目。用户的目标不是最快得到完整代码，而是亲手理解一个推理引擎如何从 CMake、基础类型、测试、backend、runtime、kernel 一步步长出来。

## 2. 当前协作模式

项目当前采用“用户手敲代码，Agent 维护路线与文档”的模式。

用户负责：

- 手动创建和修改学习路径中的代码文件。
- 运行构建、测试和示例命令。
- 把测试结果或遇到的问题反馈给 Agent。
- 在关键设计选择上做最终确认。

Agent 负责：

- 解释每个文件为什么存在、属于哪一层、暂时不负责什么。
- 给出可手敲的最小代码片段和验证命令。
- 在用户完成后读取实际文件，帮助检查问题。
- 维护 `FirstLLM.md`、`ProjectNodes.md`、`agent.md` 和 `ProgressLog.md`。
- 根据用户反馈把进度、测试结果和设计思考写入日志。

除非用户明确要求，Agent 不应直接生成或改写学习路径中的 C++ 代码文件。需要做教学演示时，应先说明“请你手敲以下内容”，而不是直接代写。

## 3. 项目定位

FirstLLM 是一个基于 C++20 和 CMake 的极简大模型推理引擎学习项目。

当前已确认方向：

- 第一批模型面向 decoder-only 文本生成模型。
- CPU 路径优先，CUDA 后续作为可选 backend 逐步设计和实现。
- 项目重视清晰性、边界感、测试和阶段性记录，优先级高于性能优化。
- 模型格式长期倾向兼容 GGUF，但不会在早期急着完整加载大模型。

核心目标：

> 先建立清晰、可扩展、可测试的 C++ 推理运行时骨架，再逐步实现 tensor、backend、kernel、model loader、KV cache、sampler 和服务化能力。

## 4. 阅读顺序

每次开始较大的工作前，按以下顺序阅读：

1. `FirstLLM.md`：项目定位、总体架构、长期路线。
2. `ProjectNodes.md`：当前章节、下一步节点、每个文件的学习目标。
3. `ProgressLog.md`：实际完成了什么、验证结果是什么、下一步是什么。
4. `agent.md`：确认协作边界和文档维护规则。
5. 相关代码文件：只阅读和当前节点有关的文件。

## 5. 架构边界

FirstLLM 按层组织：

```text
Application / Examples / Tools
        |
        v
Runtime
  - Engine
  - Scheduler          later
  - Sampler            later
        |
        v
Model Layer
  - ModelConfig        later
  - GGUF Reader        later
  - ModelLoader        later
  - KV Cache           later
        |
        v
Backend Abstraction
  - Backend
  - BackendRegistry
  - Device
  - Capability
        |
        v
Backend Implementations
  - CPU
  - CUDA               later
        |
        v
Kernels / Memory / Device APIs
```

边界规则：

- `core/` 放基础类型，例如 `Status`、`Tensor`、`Device`、`Backend`。
- `runtime/` 放用户请求、生成流程、调度、采样等运行时接口。
- `backends/` 放具体 backend 的公开接口，例如 CPU、CUDA。
- `kernels/` 放具体数值计算接口和实现。
- `model/` 放模型配置、GGUF reader、权重索引、KV cache。
- `examples/` 放学习者可运行示例。
- `tests/` 放小而明确的行为验证。

## 6. 当前进度

截至当前协作状态：

- 第 0 章最小工程已完成：`CMakeLists.txt`、基础目录结构、CMake configure 已跑通。
- 第 1 章 `Status` 模块已完成：`status.h`、`status.cpp`、`firstllm` 静态库目标。
- 第 1 章 `Status` 测试已完成：用户手动创建 `tests/status_test.cpp`，并通过 CTest。
- 下一步进入第 2 章：`Tensor` 基础类型。

当前重要代码文件：

```text
CMakeLists.txt
include/firstllm/core/status.h
src/core/status.cpp
tests/status_test.cpp
```

## 7. 标准工作流程

每个节点按以下顺序推进：

1. 说明本节点属于哪一层。
2. 说明它解决什么问题。
3. 说明它暂时不解决什么问题。
4. 用户手动创建或修改文件。
5. 用户运行最小验证命令。
6. Agent 根据用户反馈检查结果。
7. Agent 更新 `ProgressLog.md`。
8. 必要时 Agent 更新 `ProjectNodes.md` 或 `FirstLLM.md`。

测试节点同样遵守这个流程。Agent 可以给出测试代码，但默认应让用户手动创建测试文件并运行测试。

## 8. 文档维护规则

以下文档由 Agent 维护：

- `FirstLLM.md`：项目设计、长期架构、阶段范围。
- `ProjectNodes.md`：章节路线、节点状态、下一步任务。
- `agent.md`：协作协议、工作边界。
- `ProgressLog.md`：实际进度、验证结果、章节索引。

用户可以提出修改建议和方向调整，Agent 负责把这些约定写入文档。

## 9. 日志规则

`ProgressLog.md` 应按章节建立索引，方便未来检索。每条日志至少包含：

- 时间。
- 阶段。
- 完成内容。
- 新增文件或目录。
- 修改文件。
- 验证情况。
- 已知问题。
- 设计思考。
- 下一步。

如果用户完成了代码或测试，日志中应明确写出“用户手动完成”。如果 Agent 只维护日志，也应明确区分。

## 10. 沟通原则

- 先指导，再动手。
- 先小步验证，再进入下一章。
- 解释设计边界，不只给代码。
- 不为了速度跳过测试。
- 不为了完整性提前引入复杂抽象。
- 当用户指出流程问题时，优先修正文档和协作方式。
