# FirstLLM Agent 工作指南

## 1. 文件目的

本文件是 FirstLLM 的协作协议。它说明后续智能体或协作者进入项目后应该如何阅读、如何指导、如何写代码注释、如何维护文档，以及哪些事情不应该越界代办。

FirstLLM 是一个学习型 C++20 推理引擎项目。用户的目标不是最快得到完整代码，而是理解一个推理引擎如何从 CMake、基础类型、测试、backend、runtime、kernel、model loader 一步步长出来。

## 2. 当前协作模式

项目当前采用“用户手敲为主，Agent 指导与维护；用户明确要求时 Agent 可接手实现”的模式。

用户负责：

- 手动创建和修改学习路线中的代码文件。
- 运行构建、测试和示例命令。
- 把测试结果、困惑点或遇到的问题反馈给 Agent。
- 在关键设计选择上做最终确认。

Agent 负责：

- 解释每个文件为什么存在、属于哪一层、暂时不负责什么。
- 给出可手敲的最小代码片段、测试片段和验证命令。
- 在用户完成后读取实际文件，帮助检查问题。
- 维护 `FirstLLM.md`、`ProjectNodes.md`、`ProgressLog.md` 和 `agent.md`。
- 根据用户反馈把进度、测试结果和设计思考写入日志。
- 当用户明确说“帮我完成”“我迷糊了”“你来改”时，可以直接修改 C++ 代码和测试，但必须在日志中区分是用户手动完成还是 Agent 接手完成。

默认情况下，Agent 不应直接代写学习路径中的 C++ 代码。需要做教学演示时，应先说明“请你手敲以下内容”。但当用户明确要求 Agent 接手时，Agent 可以完成代码修改、测试扩展、构建验证和文档更新。

## 3. 中文注释规则

从当前阶段开始，所有新增或修改的 C++ 代码都应带中文注释，方便学习和复盘。

注释要求：

- 头文件中的类、结构体、枚举、函数和成员变量都应说明职责。
- 实现文件中的关键函数、关键局部变量和关键分支都应说明作用。
- 测试文件中的 helper 函数、测试数据、错误路径和关键断言应说明在验证什么。
- 对于功能不完善、当前只是骨架、后续要扩展的部分，必须写清楚“当前限制”和“后续扩展方向”。
- 注释应解释设计意图和边界，不要写机械重复语法的空话。
- 以后 Agent 给用户的新代码片段，必须默认包含完整中文注释。

典型注释重点：

- `Status`：说明错误码语义和失败路径。
- `Tensor`：说明 dtype、shape、raw bytes、当前不支持 stride/view/GPU memory。
- `Backend`：说明能力查询、可用性、优先级选择。
- `CUDA backend`：说明当前只是骨架，不依赖 CUDA SDK，不执行真实 GPU kernel。
- `GGUF reader`：说明当前读取到哪一层，哪些 metadata 类型暂未支持，tensor info 和权重加载何时扩展。

## 4. 项目定位

FirstLLM 是一个基于 C++20 和 CMake 的极简大模型推理引擎学习项目。

当前已确认方向：

- 第一批模型面向 decoder-only 文本生成模型。
- CPU 路径优先，CUDA 作为可选 backend 逐步设计和实现。
- 项目重视清晰性、边界感、测试、中文注释和阶段性记录，优先级高于性能优化。
- 模型格式长期倾向兼容 GGUF，但不会在早期急着完整加载大模型。

核心目标：

> 先建立清晰、可扩展、可测试、带注释的 C++ 推理运行时骨架，再逐步实现 tensor、backend、kernel、model loader、KV cache、sampler 和服务化能力。

## 5. 阅读顺序

每次开始较大的工作前，按以下顺序阅读：

1. `FirstLLM.md`：项目定位、总体架构、长期路线。
2. `ProjectNodes.md`：当前章节、下一步节点、每个文件的学习目标。
3. `ProgressLog.md`：实际完成了什么、验证结果是什么、下一步是什么。
4. `agent.md`：确认协作边界、注释要求和文档维护规则。
5. 相关代码文件：只阅读和当前节点有关的文件。

## 6. 架构边界

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
  - GGUF Reader
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
  - CUDA skeleton
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

## 7. 当前进度

截至当前协作状态：

- 第 0 章：最小 CMake 工程已完成。
- 第 1 章：`Status` 错误处理已完成。
- 第 2 章：`Tensor` 基础类型已完成。
- 第 3 章：`Backend` 抽象和 `BackendRegistry` 已完成。
- 第 4 章：`CPU Backend` 已完成。
- 第 5 章：`Engine Runtime` 已完成。
- 第 6 章：总入口、示例程序和 smoke test 已完成。
- 第 7 章：CPU add 算子已完成。
- 第 8 章：CPU matmul 算子已完成。
- 第 9 章：CPU softmax 和 RMSNorm 已完成。
- 第 10 章：CUDA backend 骨架已完成。
- 第 11 章：GGUF reader 首版已完成，已完成 header、metadata key/value、tensor info、data section 对齐和 tensor 数据绝对偏移计算。
- 第 12 章：Tiny Llama-like forward 首版已完成，已能串联 RMSNorm、MatMul、Add bias 和 Softmax。
- 第 13 章：KV cache 与自回归生成首版已完成，已完成 KV cache、greedy next token 和自回归单步生成接口，下一步进入 tokenizer、sampler 和真实文本生成准备。

当前验证状态：

```text
CTest: 100% tests passed, 0 tests failed out of 15
```

已知环境现象：

- Codex 沙箱内直接运行 MSBuild 时，可能在 `ZERO_CHECK.vcxproj` 的 FileTracker 阶段遇到 `拒绝访问`。
- 遇到该问题时，用同一条构建命令提权重跑通常可以通过；这不是当前 C++ 代码错误。

## 8. 标准工作流程

每个节点按以下顺序推进：

1. 说明本节点属于哪一层。
2. 说明它解决什么问题。
3. 说明它暂时不解决什么问题。
4. 给出带中文注释的最小代码或测试片段。
5. 用户手动创建或修改文件。
6. 用户运行最小验证命令。
7. Agent 根据用户反馈检查结果。
8. Agent 更新 `ProgressLog.md`。
9. 必要时 Agent 更新 `ProjectNodes.md`、`FirstLLM.md` 或 `agent.md`。

如果用户明确要求 Agent 接手，则流程调整为：

1. Agent 先说明将修改哪些文件和原因。
2. Agent 直接修改代码、测试或文档。
3. Agent 运行 configure、build 和 CTest。
4. Agent 在 `ProgressLog.md` 中写明“Agent 接手完成”。
5. Agent 给用户总结变更、验证结果和下一步。

## 9. 文档维护规则

以下文档由 Agent 维护：

- `FirstLLM.md`：项目设计、长期架构、阶段范围。
- `ProjectNodes.md`：章节路线、节点状态、下一步任务。
- `agent.md`：协作协议、工作边界、注释规则。
- `ProgressLog.md`：实际进度、验证结果、章节索引。

用户可以提出修改建议和方向调整，Agent 负责把这些约定写入文档。

## 10. 日志规则

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

如果用户完成了代码或测试，日志中应明确写出“用户已手动完成”。如果 Agent 接手完成代码、测试或整理，也必须明确写出“Agent 接手完成”。

## 11. 沟通原则

- 先解释边界，再进入代码。
- 先小步验证，再进入下一节。
- 给代码时必须带中文注释。
- 不为了速度跳过测试。
- 不为了完整性提前引入复杂抽象。
- 当用户指出流程问题时，优先修正文档和协作方式。
- 当用户感到迷糊时，可以降低节奏、补上下文，必要时由 Agent 接手整理一小段闭环。
