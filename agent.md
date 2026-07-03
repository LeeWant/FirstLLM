# FirstLLM Agent 工作指南

## 1. 文件目的

本文件用于指导参与 FirstLLM 项目的智能体或协作者如何工作。它不是项目设计文档本身，而是“进入项目后的工作协议”。

当你开始处理 FirstLLM 的任何任务时，应先阅读并遵守本文，再根据任务需要查看其他文档和代码。

## 2. 项目定位

FirstLLM 是一个基于 C++ 的极简大模型推理引擎学习项目。

当前已确认方向：

- 第一批模型支持 decoder-only 文本生成模型。
- 后续预留多模态扩展空间。
- 硬件路线为 CPU 优先，同时 CUDA 同步设计和逐步实现。
- 项目更偏学习型代码，清晰性、可解释性和阶段性验证优先于极限性能。
- 模型格式直接兼容 GGUF。

核心目标：

> 先建立清晰、可扩展、可测试的推理引擎骨架，再逐步实现算子、模型加载、KV cache、采样、CUDA backend 和服务化能力。

## 3. 进入项目后的阅读顺序

每次开始较大的修改前，应按以下顺序理解项目：

1. `FirstLLM.md`：了解项目总目标、架构分层和设计决策。
2. `ProjectNodes.md`：了解当前阶段、章节规划和后续实现顺序。
3. `ProgressLog.md`：了解实际进度、已完成工作、已知问题和下一步。
4. `agent.md`：确认工作方式、文件组织和进度记录要求。
5. 相关代码文件：只阅读与当前任务相关的模块，避免无关重构。

## 4. 项目架构原则

FirstLLM 的架构按层组织：

```text
Application / Examples / Tools
        |
        v
Runtime
  - Engine
  - Generation
  - Scheduler          later
  - Sampler            later
        |
        v
Model Layer
  - ModelConfig        later
  - GGUF Reader        later
  - ModelLoader        later
  - Llama-like Model   later
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

设计边界：

- `Engine` 是用户侧入口，负责编排，不应直接实现算子或解析 GGUF。
- `Backend` 负责声明设备、可用性和 capability，不应包含模型业务逻辑。
- `Kernel` 负责具体数值计算，不应决定请求调度。
- `Model Layer` 负责模型结构、权重索引和 GGUF 解析，不应管理用户请求队列。
- CPU backend 是 correctness baseline，CUDA backend 的结果应与 CPU 对照。

## 5. 当前文件结构

```text
FirstLLM/
  agent.md
  FirstLLM.md
  ProjectNodes.md
  ProgressLog.md
  EnvironmentSetup.md
  scripts/
    check-env.ps1
    firstllm-env.ps1
```

当前仓库处于“从零手敲学习模式”。此前由智能体生成的代码骨架、构建配置和构建产物已经清理，用户将按照 `ProjectNodes.md` 手动重建每个文件。

后续由用户按章节手动新增：

```text
CMakeLists.txt
include/
src/
examples/
tests/
```

## 6. 文件职责规则

新增文件时，应遵守以下职责边界：

- `include/firstllm/core/`：基础类型，例如 `Status`、`Tensor`、`Device`、`Backend`。
- `include/firstllm/runtime/`：用户请求、生成流程、调度器、采样器等 runtime 层接口。
- `include/firstllm/backends/`：具体 backend 的 public 接口，例如 CPU、CUDA。
- `include/firstllm/kernels/`：算子接口，例如 add、matmul、softmax。
- `include/firstllm/model/`：模型配置、GGUF reader、权重索引、KV cache、Llama-like 模型。
- `include/firstllm/tokenizer/`：tokenizer 抽象与实现入口。
- `src/`：与 include 对应的实现文件。
- `examples/`：面向学习者的最小可运行示例。
- `tests/`：验证核心行为的小测试。
- `tools/`：辅助检查工具，例如 GGUF inspect。
- `docs/`：术语、学习笔记、设计说明。

## 7. 编码工作原则

FirstLLM 是学习型项目，因此代码应优先做到：

- 清晰：先写容易理解的版本，再谈优化。
- 可测：每个核心功能都要有小测试。
- 可解释：重要接口要有简短注释说明用途和边界。
- 小步提交：一次任务只推进一个清晰节点。
- 少依赖：第一阶段尽量只使用 C++ 标准库。
- 不过早抽象：只有当重复和复杂度真的出现时再抽象。

新增模块时，应优先采用以下顺序：

1. 写接口。
2. 写最小实现。
3. 写测试或 example。
4. 更新 `ProjectNodes.md` 中对应节点状态或说明。
5. 更新 `ProgressLog.md`。

## 8. 进度管理规则

`ProgressLog.md` 是项目过程记录文件。每完成一个明确任务，都应追加一条日志。

日志必须记录：

- 时间。
- 当前阶段。
- 完成内容。
- 新增文件。
- 修改文件。
- 验证情况。
- 已知问题 / Bug。
- 设计思考。
- 下一步。

如果任务只是很小的文本修正，也可以简化记录，但涉及架构、代码、测试、目录结构或路线调整时必须记录。

追加日志时不要覆盖历史记录。新的日志应放在 `## 3. 项目日志` 下方，按时间从旧到新排列。

## 9. 当前推荐下一步

根据当前进度，下一步应进入 M1：

1. 按 `ProjectNodes.md` 第 0 章手动创建 `CMakeLists.txt`。
2. 手动创建 `include/`、`src/`、`examples/`、`tests/` 等空目录。
3. 按第 1 章开始手敲 `Status` 模块。
4. 每完成一个文件或节点，都更新 `ProgressLog.md`。

不要立刻恢复完整代码骨架，也不要一次性生成所有文件。项目当前最重要的是让用户亲手理解每个文件为什么存在。

## 10. 当前环境状态

截至 `2026-07-03 15:01:45 +08:00`，FirstLLM 的 Windows 开发环境已经完成基础配置：

- Git 已可通过项目环境脚本识别。
- CMake 已安装并可通过项目环境脚本识别。
- Visual Studio 2022 Build Tools 已安装，MSVC `cl` 可通过项目环境脚本识别。
- Ninja 可通过 Visual Studio Build Tools 附带路径识别。
- CMake configure、build、ctest 和示例程序运行均已验证通过。
- CUDA / `nvcc` 尚未安装；当前阶段不强制需要。

后续执行构建前，建议先运行：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; .\scripts\check-env.ps1"
```

如果普通 PowerShell 找不到工具，优先使用 `scripts/firstllm-env.ps1` 刷新当前会话环境，而不是直接判断工具未安装。

## 11. 与用户沟通方式

用户正在学习推理引擎，表达可能是高层、模糊或探索性的。协作者应主动补足结构，但不要跳过解释。

工作时应做到：

- 先把模糊目标拆成清晰节点。
- 不确定但不阻塞的问题，可以先做合理假设并记录。
- 会影响路线的关键问题，应向用户确认。
- 每个新增设计都解释“为什么需要它”。
- 每次完成后给出短小、明确的成果总结和下一步建议。
