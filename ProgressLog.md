# FirstLLM 进度管理日志

## 1. 文件用途

本文件记录 FirstLLM 的实际推进过程。它回答：

- 已经完成了什么？
- 哪些命令验证过？
- 当前卡点是什么？
- 下一步应该做什么？

`FirstLLM.md` 负责项目设计，`ProjectNodes.md` 负责章节路线，`agent.md` 负责协作协议，本文件负责事实记录和检索。

## 2. 章节索引

| 章节 / 主题 | 当前状态 | 关键记录 |
| --- | --- | --- |
| 项目方向与环境 | 已完成基础配置 | 2026-07-03 14:32:57 到 15:09:16 |
| 第 0 章：重新建立最小工程 | 已完成 | 2026-07-03 16:13:19 |
| 第 1 章：Status 错误处理 | 已完成 | 2026-07-03 17:24:39 |
| 第 2 章：Tensor 基础类型 | 已完成 | 2026-07-06 11:02:33 |
| 第 3 章：Backend 抽象 | 已完成 | 2026-07-06 14:00:30 |
| 文档与协作流程 | 已更新 | 2026-07-03 17:32:18 |

后续新增日志时，应放到对应章节下，并同步更新本索引。

## 3. 日志模板

```text
时间：
章节 / 阶段：
完成内容：
新增文件 / 目录：
修改文件：
验证情况：
已知问题 / Bug：
设计思考：
下一步：
```

## 4. 项目方向与环境日志

### 2026-07-03 14:32:57 +08:00

章节 / 阶段：项目方向确认与初始骨架

完成内容：

- 明确 FirstLLM 是基于 C++ 的极简大模型推理引擎学习项目。
- 确认优先支持 decoder-only 文本生成模型。
- 确认硬件路线为 CPU 优先，CUDA 后续逐步设计和实现。
- 确认项目偏学习型代码，强调清晰性、文档、注释和阶段性测试。
- 确认模型格式长期倾向 GGUF。
- 参考 ONNX Runtime、TensorRT-LLM、vLLM、llama.cpp 的公开架构，整理初始设计。
- 创建项目主设计文档和初始代码骨架。

新增文件：

- `FirstLLM.md`
- `ProjectNodes.md`
- `ProgressLog.md`
- 初始 C++ 骨架文件若干

验证情况：

- 当时环境缺少 CMake、C++ 编译器和可用 Git PATH，无法完成真实构建。

已知问题 / Bug：

- 后续已切换到“从零手敲学习模式”，该批初始代码骨架已清理。

设计思考：

- 推理引擎不应从完整大模型开始，而应先建立 runtime、backend、tensor、status、测试和文档地基。

下一步：

- 准备环境并调整为适合学习的手敲路线。

### 2026-07-03 14:38:17 +08:00

章节 / 阶段：协作规范补充

完成内容：

- 新增 `agent.md`，记录项目协作方式和文件职责。
- 明确文档阅读顺序和进度记录要求。

新增文件：

- `agent.md`

修改文件：

- `ProgressLog.md`

验证情况：

- 文档已创建；该节点不涉及代码构建。

下一步：

- 完成开发环境配置。

### 2026-07-03 14:41:35 +08:00

章节 / 阶段：环境搭建准备

完成内容：

- 新增 `EnvironmentSetup.md`。
- 新增 `scripts/check-env.ps1`。
- 新增 `scripts/firstllm-env.ps1`。
- 探测当前机器上的 Git、CMake、MSVC、Ninja、CUDA 状态。

新增文件：

- `EnvironmentSetup.md`
- `scripts/check-env.ps1`
- `scripts/firstllm-env.ps1`

验证情况：

- 发现 Git 已安装但不在当前 PATH。
- 当时未发现 CMake、MSVC `cl`、CUDA `nvcc`。
- 发现 `winget` 可用。

下一步：

- 安装或补齐 CMake 和 Visual Studio Build Tools。

### 2026-07-03 14:41:35 +08:00 后续环境安装记录

章节 / 阶段：环境安装执行

完成内容：

- 通过 `winget` 安装 CMake。
- 通过 `winget` 安装 Visual Studio 2022 Build Tools 和 C++ workload。
- 修正 `scripts/firstllm-env.ps1` 中 CUDA 版本目录处理方式。
- 更新 `EnvironmentSetup.md` 的 PowerShell 执行策略说明。

修改文件：

- `EnvironmentSetup.md`
- `scripts/firstllm-env.ps1`
- `ProgressLog.md`

验证情况：

- CMake 安装命令返回成功。
- Visual Studio Build Tools 安装命令返回成功。

已知问题 / Bug：

- 当前 shell 需要刷新 PATH 或运行环境脚本后才能找到新工具。

下一步：

- 加载环境脚本并验证工具链。

### 2026-07-03 15:01:45 +08:00

章节 / 阶段：环境搭建完成与构建验证

完成内容：

- 确认 `scripts/firstllm-env.ps1` 可加载。
- 确认 Git、CMake、MSVC `cl` 和 Ninja 可用。
- 成功执行 CMake configure、build、ctest 和示例程序。
- 更新环境文档和协作说明。

修改文件：

- `EnvironmentSetup.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- `git --version` 输出 `git version 2.55.0.windows.1`。
- `cmake --version` 输出 `cmake version 4.3.4`。
- CMake 识别 MSVC 19.44.35228.0 和 Windows SDK 10.0.26100.0。
- 当时初始骨架通过 `ctest`，1 个测试全部通过。

已知问题 / Bug：

- CUDA / `nvcc` 未安装，当前阶段不需要。
- 普通 PowerShell 可能仍需 `-ExecutionPolicy Bypass`。

下一步：

- 切换到更适合学习的从零手敲模式。

### 2026-07-03 15:09:16 +08:00

章节 / 阶段：切换到从零手敲学习模式

完成内容：

- 根据用户要求，清理此前由 Agent 生成的 C++ 代码骨架和构建产物。
- 删除旧的 `include/`、`src/`、`examples/`、`tests/`、`build/` 和 `CMakeLists.txt`。
- 保留项目文档、环境指南和环境脚本。
- 重写 `ProjectNodes.md`，改为从第一个文件开始的手敲学习路线。
- 更新 `agent.md`，匹配新的协作方式。

修改文件：

- `ProjectNodes.md`
- `agent.md`
- `ProgressLog.md`

删除文件 / 目录：

- `CMakeLists.txt`
- `include/`
- `src/`
- `examples/`
- `tests/`
- `build/`

验证情况：

- 确认清理目标均位于 `D:\workplace\FirstLLM` 工作区内。

设计思考：

- 对学习型项目来说，亲手敲每个文件比直接拥有完整代码更重要。

下一步：

- 从第 0 章重新建立最小工程。

## 5. 第 0 章：重新建立最小工程

### 2026-07-03 16:13:19 +08:00

章节 / 阶段：第 0 章 重新建立最小工程

完成内容：

- 补全 `CMakeLists.txt`，写入最小 CMake 项目声明。
- 声明 FirstLLM 是 C++ 项目，版本为 `0.1.0`。
- 预留 `FIRSTLLM_BUILD_EXAMPLES` 和 `FIRSTLLM_BUILD_TESTS` 构建开关。
- 创建基础目录结构。
- 执行 CMake configure，验证最小工程声明可被 CMake 读取。

新增目录：

- `include/`
- `include/firstllm/`
- `include/firstllm/core/`
- `include/firstllm/runtime/`
- `include/firstllm/backends/`
- `src/`
- `src/core/`
- `src/runtime/`
- `src/backends/`
- `src/backends/cpu/`
- `examples/`
- `tests/`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- 生成 `build/` 构建目录。
- 当时还没有源码，因此只验证 configure。

设计思考：

- `CMakeLists.txt` 第一版只负责声明项目和构建开关。
- 目录结构先行，让接口、实现、示例和测试自然分层。

下一步：

- 进入第 1 章，创建 `Status` 模块。

## 6. 第 1 章：Status 错误处理

### 2026-07-03 17:24:39 +08:00

章节 / 阶段：第 1 章 Status 错误处理与测试

完成内容：

- 用户已手动创建 `include/firstllm/core/status.h`。
- 用户已手动创建 `src/core/status.cpp`。
- 用户已将 `src/core/status.cpp` 接入 `firstllm` 静态库目标。
- 用户手动创建 `tests/status_test.cpp`。
- 用户在测试中验证 `Status::Ok()` 的成功状态行为。
- 用户在测试中验证 `Status::InvalidArgument()` 的错误码与错误信息。
- 用户将 `firstllm_status_test` 接入 CMake 和 CTest。
- Agent 根据用户反馈记录测试结果。

新增文件：

- `include/firstllm/core/status.h`
- `src/core/status.cpp`
- `tests/status_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 1`。
- 总测试时间为 `2.38 sec`。

已知问题 / Bug：

- 当前测试只覆盖 `Ok` 和 `InvalidArgument` 两类状态。
- `NotFound`、`BackendUnavailable` 和 `Internal` 后续可以在需要时补充测试。

设计思考：

- 这是 FirstLLM 第一次完成“模块代码 + 测试代码 + CMake 接入 + CTest 验证”的闭环。
- 第一版测试使用标准库 `assert`，保持依赖最少。
- 用户手动完成测试代码，有助于理解规范化开发流程。

下一步：

- 进入第 2 章，创建 `Tensor` 基础类型。

## 7. 第 2 章：Tensor 基础类型

### 2026-07-06 11:02:33 +08:00

章节 / 阶段：第 2 章 Tensor 基础类型与测试

完成内容：

- 用户已手动创建 `include/firstllm/core/tensor.h`。
- 用户已手动创建 `src/core/tensor.cpp`。
- 用户已将 `src/core/tensor.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/tensor_test.cpp`。
- 用户已将 `firstllm_tensor_test` 接入 CMake 和 CTest。
- `TensorShape` 已支持 rank、dim 和元素数量查询。
- `Tensor` 已支持 dtype、shape、元素数量、字节数量和 raw byte data 访问。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认测试闭环通过。

新增文件：

- `include/firstllm/core/tensor.h`
- `src/core/tensor.cpp`
- `tests/tensor_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 2`。
- `firstllm_status_test` 与 `firstllm_tensor_test` 均通过。
- 总测试时间为 `0.03 sec`。

已知问题 / Bug：

- 在 Codex 沙箱内执行 MSBuild 时，曾遇到 MSBuild 文件跟踪阶段 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `TensorShape::num_elements()` 对空 shape 返回 `1`，这符合 scalar tensor 的常见约定；后续如需要可专门补充 scalar 测试。

设计思考：

- 第一版 `Tensor` 只表达 `dtype + shape + host raw bytes`，不提前引入 GPU memory、stride、view 或量化布局。
- 先让数据容器稳定，后续 CPU 算子、backend 和 runtime 才有统一的数据接口。
- `DTypeSize()`、`TensorShape::num_elements()` 和 `Tensor::byte_size()` 建立了 shape、dtype 与内存大小之间的基本关系。

下一步：

- 进入第 3 章，创建 `Backend` 抽象。

## 8. 第 3 章：Backend 抽象

### 2026-07-06 14:00:30 +08:00

章节 / 阶段：第 3 章 Backend 抽象与测试

完成内容：

- 用户已手动创建 `include/firstllm/core/backend.h`。
- 用户已手动创建 `src/core/backend.cpp`。
- 用户已将 `src/core/backend.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/backend_test.cpp`。
- 用户已将 `firstllm_backend_test` 接入 CMake 和 CTest。
- `Backend` 抽象已表达 backend 信息、初始化、可用性和能力查询。
- `BackendRegistry` 已支持添加 backend、列出 backend，并按 op、dtype、可用性和优先级查找 backend。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认测试闭环通过。

新增文件：

- `include/firstllm/core/backend.h`
- `src/core/backend.cpp`
- `tests/backend_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 3`。
- `firstllm_status_test`、`firstllm_tensor_test` 与 `firstllm_backend_test` 均通过。
- 总测试时间为 `0.03 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `Backend` 只描述能力和选择逻辑，不执行真实算子。

设计思考：

- `Backend` 抽象回答“我是谁、我是否可用、我支持什么”，不承担具体数值计算。
- `BackendRegistry::find_backend()` 让 runtime 后续可以从多个 backend 中选择最合适的一个。
- 真实 CPU 算子和后端实现放到后续章节，避免把抽象层和具体实现混在一起。

下一步：

- 进入第 4 章，创建 `CPU Backend`。

## 9. 文档与协作流程日志

### 2026-07-03 17:32:18 +08:00

章节 / 阶段：文档结构与协作流程更新

完成内容：

- 根据用户反馈，明确 `FirstLLM.md`、`ProjectNodes.md`、`agent.md` 由 Agent 维护。
- 明确 `ProgressLog.md` 也由 Agent 根据用户完成情况进行记录。
- 更新 `agent.md`，写入“用户手敲代码，Agent 维护路线与文档”的协作模式。
- 更新 `ProjectNodes.md`，加入当前章节状态、已完成节点和下一步。
- 更新 `FirstLLM.md`，同步当前项目阶段、已完成内容和协作方式。
- 重构 `ProgressLog.md`，增加章节索引，并按主题和章节组织历史记录。

修改文件：

- `agent.md`
- `ProjectNodes.md`
- `FirstLLM.md`
- `ProgressLog.md`

验证情况：

- 文档已按当前沟通流程更新。
- 本次不涉及 C++ 源码构建。

设计思考：

- 文档本身也是项目结构的一部分。把协作方式写清楚，可以减少后续反复解释和误会。
- 日志按章节索引后，更适合长期学习项目检索。

下一步：

- 从第 2 章开始，继续用“用户手敲、Agent 指导、Agent 记录”的方式推进 `Tensor`。
