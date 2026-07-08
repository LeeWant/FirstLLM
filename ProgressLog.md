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
| 第 4 章：CPU Backend | 已完成 | 2026-07-06 14:24:24 |
| 第 5 章：Engine Runtime | 已完成 | 2026-07-06 14:55:20 |
| 第 6 章：示例程序和 smoke test | 已完成 | 2026-07-06 15:16:32 |
| 第 7 章：CPU add 算子 | 已完成 | 2026-07-06 16:13:55 |
| 第 8 章：CPU matmul 算子 | 已完成 | 2026-07-06 16:31:05 |
| 第 9 章：softmax 与 rms_norm | 已完成 | 2026-07-06 17:07:07 |
| 第 10 章：CUDA backend 骨架 | 已完成 | 2026-07-06 17:40:50 |
| 第 11 章：GGUF reader | 已完成首版 | 2026-07-08 15:33:02 data section 与 tensor 绝对偏移已完成 |
| 第 12 章：Tiny Llama-like forward | 已完成首版 | 2026-07-08 16:10:01 forward 闭环已完成 |
| 第 13 章：KV cache 与自回归生成 | 已完成首版 | 2026-07-08 17:29:58 单步生成接口已完成 |
| 文档与协作流程 | 已更新 | 2026-07-07 09:33:13 中文注释与工作流更新 |

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

## 9. 第 4 章：CPU Backend

### 2026-07-06 14:24:24 +08:00

章节 / 阶段：第 4 章 CPU Backend 与测试

完成内容：

- 用户已手动创建 `include/firstllm/backends/cpu_backend.h`。
- 用户已手动创建 `src/backends/cpu/cpu_backend.cpp`。
- 用户已将 `src/backends/cpu/cpu_backend.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/cpu_backend_test.cpp`。
- 用户已将 `firstllm_cpu_backend_test` 接入 CMake 和 CTest。
- `CpuBackend` 已能返回 backend 信息。
- `CpuBackend` 已支持初始化前不可用、初始化后可用的状态行为。
- `CpuBackend` 当前只声明支持 `float32 add`。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认测试闭环通过。

新增文件：

- `include/firstllm/backends/cpu_backend.h`
- `src/backends/cpu/cpu_backend.cpp`
- `tests/cpu_backend_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 4`。
- `firstllm_status_test`、`firstllm_tensor_test`、`firstllm_backend_test` 与 `firstllm_cpu_backend_test` 均通过。
- 总测试时间为 `0.04 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `CpuBackend` 只声明能力，不执行真实 `add` 算子。

设计思考：

- CPU backend 是第一个真实 backend，但这一章仍只做能力声明和可用性验证。
- 当前只声明支持 `float32 add`，避免提前让 registry 认为 `matmul`、`softmax` 或其他 dtype 已经可用。
- 真实数值计算应在后续 kernel / 算子章节中接入。

下一步：

- 进入第 5 章，创建 `Engine Runtime`。

## 10. 第 5 章：Engine Runtime

### 2026-07-06 14:55:20 +08:00

章节 / 阶段：第 5 章 Engine Runtime 与测试

完成内容：

- 用户已手动创建 `include/firstllm/runtime/engine.h`。
- 用户已手动创建 `src/runtime/engine.cpp`。
- 用户已将 `src/runtime/engine.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/engine_test.cpp`。
- 用户已将 `firstllm_engine_test` 接入 CMake 和 CTest。
- `EngineConfig` 已支持控制是否启用 CPU backend。
- `Engine` 已支持初始化 backend registry。
- `Engine` 已支持通过 op 和 dtype 查找 backend。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认测试闭环通过。

新增文件：

- `include/firstllm/runtime/engine.h`
- `src/runtime/engine.cpp`
- `tests/engine_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 5`。
- `firstllm_status_test`、`firstllm_tensor_test`、`firstllm_backend_test`、`firstllm_cpu_backend_test` 与 `firstllm_engine_test` 均通过。
- 总测试时间为 `0.05 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `Engine` 只负责编排和 backend 查找，不执行真实算子、不解析模型、不管理 KV cache。

设计思考：

- `Engine` 是用户侧入口，但第一版只做最小编排：根据配置初始化 CPU backend，并通过 registry 查询能力。
- 这一步把 `BackendRegistry` 和 `CpuBackend` 接到了 runtime 层，形成从用户入口到 backend 选择的第一条路径。
- 真实请求结构、模型加载和推理循环应留到后续章节逐步加入。

下一步：

- 进入第 6 章，创建总入口、示例程序和 smoke test。

## 11. 第 6 章：示例程序和 smoke test

### 2026-07-06 15:16:32 +08:00

章节 / 阶段：第 6 章 总入口、示例程序和 smoke test

完成内容：

- 用户已手动创建 `include/firstllm/firstllm.h`。
- 用户已手动创建 `examples/firstllm_info.cpp`。
- 用户已将 `firstllm_info` 示例程序接入 CMake。
- 用户已手动创建 `tests/smoke.cpp`。
- 用户已将 `firstllm_smoke_test` 接入 CMake 和 CTest。
- 总入口 header 已能统一包含 Status、Tensor、Backend、CpuBackend 和 Engine。
- 示例程序已能初始化 Engine 并打印选中的 backend。
- smoke test 已能通过统一 header 走通 Engine、Backend 和 Tensor 基础路径。
- Agent 检查文件后重新执行 configure、build、ctest 和示例程序，确认闭环通过。

新增文件：

- `include/firstllm/firstllm.h`
- `examples/firstllm_info.cpp`
- `tests/smoke.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 6`。
- `firstllm_status_test`、`firstllm_tensor_test`、`firstllm_backend_test`、`firstllm_cpu_backend_test`、`firstllm_engine_test` 与 `firstllm_smoke_test` 均通过。
- 总测试时间为 `0.08 sec`。
- `firstllm_info.exe` 运行成功，输出 `FirstLLM initialized` 和 `Selected backend: cpu`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前示例只验证 runtime 和 backend 选择路径，不执行真实数值算子。

设计思考：

- `firstllm/firstllm.h` 作为统一 public header，减少外部用户需要知道的内部头文件路径。
- `firstllm_info` 是给人直接运行看的示例，`smoke.cpp` 是给 CTest 自动验证的系统级小闭环。
- 第 6 章完成后，项目已经有从 public header 到 Engine、Backend、Tensor 的最小可用路径。

下一步：

- 进入第 7 章，创建 CPU add 算子。

## 12. 第 7 章：CPU add 算子

### 2026-07-06 16:13:55 +08:00

章节 / 阶段：第 7 章 CPU add 算子与测试

完成内容：

- 用户已手动创建 `include/firstllm/kernels/cpu/add.h`。
- 用户已手动创建 `src/kernels/cpu/add.cpp`。
- 用户已将 `src/kernels/cpu/add.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/cpu_add_test.cpp`。
- 用户已将 `firstllm_cpu_add_test` 接入 CMake 和 CTest。
- `CpuAdd` 已支持 `float32` tensor 逐元素相加。
- `CpuAdd` 已检查输出指针为空、dtype 不匹配和 shape 不匹配等错误路径。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认测试闭环通过。

新增文件：

- `include/firstllm/kernels/cpu/add.h`
- `src/kernels/cpu/add.cpp`
- `tests/cpu_add_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 7`。
- `firstllm_status_test`、`firstllm_tensor_test`、`firstllm_backend_test`、`firstllm_cpu_backend_test`、`firstllm_engine_test`、`firstllm_smoke_test` 与 `firstllm_cpu_add_test` 均通过。
- 总测试时间为 `0.10 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `CpuAdd` 只支持 `float32`，不支持广播、stride、view 或自动创建输出 tensor。

设计思考：

- `CpuAdd` 是 FirstLLM 的第一个真实数值算子，开始把 `Tensor`、`Status` 和 CPU 路径连起来。
- 当前实现继续使用 raw bytes 和 `std::memcpy` 读写 float，符合第一版 `Tensor` 的内存模型。
- 输出 tensor 由调用者传入，可以让算子暂时不承担内存规划职责。

下一步：

- 进入第 8 章，创建 CPU matmul 算子。

## 13. 第 8 章：CPU matmul 算子

### 2026-07-06 16:31:05 +08:00

章节 / 阶段：第 8 章 CPU matmul 算子与测试

完成内容：

- 用户已手动创建 `include/firstllm/kernels/cpu/matmul.h`。
- 用户已手动创建 `src/kernels/cpu/matmul.cpp`。
- 用户已将 `src/kernels/cpu/matmul.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/cpu_matmul_test.cpp`。
- 用户已将 `firstllm_cpu_matmul_test` 接入 CMake 和 CTest。
- `CpuMatMul` 已支持二维 `float32` tensor 矩阵乘法。
- `CpuMatMul` 已检查输出指针为空、dtype 不匹配、rank 非二维、内维度不匹配和输出 shape 错误等路径。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认测试闭环通过。

新增文件：

- `include/firstllm/kernels/cpu/matmul.h`
- `src/kernels/cpu/matmul.cpp`
- `tests/cpu_matmul_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 8`。
- `firstllm_status_test`、`firstllm_tensor_test`、`firstllm_backend_test`、`firstllm_cpu_backend_test`、`firstllm_engine_test`、`firstllm_smoke_test`、`firstllm_cpu_add_test` 与 `firstllm_cpu_matmul_test` 均通过。
- 总测试时间为 `7.25 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `CpuMatMul` 只支持二维 `float32`，不支持 batch、转置、stride、view 或优化 kernel。

设计思考：

- `CpuMatMul` 是第一个二维结构算子，重点验证 shape 语义和 row-major 索引关系。
- 当前实现使用三层循环，优先保证清晰正确，而不是追求性能。
- 与 `CpuAdd` 一样，输出 tensor 由调用者传入，算子只负责参数检查和数值计算。

下一步：

- 进入第 9 章，创建 softmax 与 rms_norm。

## 14. 第 9 章：softmax 与 rms_norm

### 2026-07-06 16:54:02 +08:00

章节 / 阶段：第 9 章 softmax 节点

完成内容：

- 用户已手动创建 `include/firstllm/kernels/cpu/softmax.h`。
- 用户已手动创建 `src/kernels/cpu/softmax.cpp`。
- 用户已将 `src/kernels/cpu/softmax.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/cpu_softmax_test.cpp`。
- 用户已将 `firstllm_cpu_softmax_test` 接入 CMake 和 CTest。
- `CpuSoftmaxLastDim` 已支持对最后一维执行 `float32` softmax。
- `CpuSoftmaxLastDim` 已使用 max-subtraction 处理数值稳定性。
- 测试已覆盖普通 softmax 结果、每行归一化、`1000` 输入的稳定性，以及空输出、dtype 错误和 shape 错误路径。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认 softmax 节点通过。

新增文件：

- `include/firstllm/kernels/cpu/softmax.h`
- `src/kernels/cpu/softmax.cpp`
- `tests/cpu_softmax_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 9`。
- `firstllm_cpu_softmax_test` 与此前全部测试均通过。
- 总测试时间为 `0.09 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `CpuSoftmaxLastDim` 只支持 `float32`，不支持 mask、temperature、in-place API 或非连续内存布局。

设计思考：

- softmax 是 logits 到概率分布的基础算子，后续 sampler 会依赖这一层语义。
- max-subtraction 是必要的数值稳定处理，避免直接计算 `exp(1000)` 这类溢出场景。
- 当前接口继续沿用调用者提供 output tensor 的模式，保持内存规划职责清楚。

下一步：

- 继续第 9 章，创建 RMSNorm。

### 2026-07-06 17:07:07 +08:00

章节 / 阶段：第 9 章 RMSNorm 节点

完成内容：

- 用户已手动创建 `include/firstllm/kernels/cpu/rms_norm.h`。
- 用户已手动创建 `src/kernels/cpu/rms_norm.cpp`。
- 用户已将 `src/kernels/cpu/rms_norm.cpp` 接入 `firstllm` 静态库目标。
- 用户已手动创建 `tests/cpu_rms_norm_test.cpp`。
- 用户已将 `firstllm_cpu_rms_norm_test` 接入 CMake 和 CTest。
- `CpuRmsNorm` 已支持二维 `float32` input、一维 `float32` weight 和同 shape output。
- `CpuRmsNorm` 按行计算 `mean(x^2)`，再用 `1 / sqrt(mean_square + epsilon)` 进行归一化，并乘以每列 weight。
- 测试已覆盖正常输出、空 output、dtype 错误、input rank 错误、weight shape 错误和 output shape 错误路径。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认第 9 章完整通过。

新增文件：

- `include/firstllm/kernels/cpu/rms_norm.h`
- `src/kernels/cpu/rms_norm.cpp`
- `tests/cpu_rms_norm_test.cpp`

修改文件：

- `CMakeLists.txt`
- `agent.md`
- `ProgressLog.md`
- `ProjectNodes.md`
- `FirstLLM.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 10`。
- `firstllm_cpu_rms_norm_test` 与此前全部测试均通过。
- 总测试时间为 `0.14 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 `CpuRmsNorm` 只支持二维连续 `float32` input 和一维 `float32` weight，不支持 batch 之外的更高 rank、half/bfloat16、in-place API 或 GPU memory。

设计思考：

- RMSNorm 是 Llama-like decoder block 的基础归一化算子，后续 attention 和 MLP 前后会依赖这一层语义。
- 当前实现继续沿用调用者提供 output tensor 的模式，保持 kernel 只负责参数检查和数值计算。
- 使用 `std::memcpy` 读写 float，继续匹配当前 `Tensor` 的 raw bytes 内存模型。

下一步：

- 进入第 10 章，创建 CUDA backend 骨架。

## 15. 第 10 章：CUDA backend 骨架

### 2026-07-06 17:40:50 +08:00

章节 / 阶段：第 10 章 CUDA backend 骨架

完成内容：

- 用户已手动创建 `include/firstllm/backends/cuda_backend.h`。
- 用户已手动创建 `src/backends/cuda/cuda_backend.cpp`。
- 用户已手动创建 `tests/cuda_backend_test.cpp`。
- 用户已将 `src/backends/cuda/cuda_backend.cpp` 接入 `firstllm` 静态库目标。
- 用户已将 `firstllm_cuda_backend_test` 接入 CMake 和 CTest。
- `CudaBackend` 已能返回 backend 信息：名称为 `cuda`，设备类型为 `DeviceType::kCuda`，优先级为 `10`。
- 当前 build 下 `CudaBackend::initialize()` 返回 `BackendUnavailable`，并保持 `is_available()` 为 false。
- 当前 `CudaBackend::supports()` 对所有 op 和 dtype 返回 false，避免 runtime 误选一个不可用 backend。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认第 10 章通过。

新增文件：

- `include/firstllm/backends/cuda_backend.h`
- `src/backends/cuda/cuda_backend.cpp`
- `tests/cuda_backend_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`
- `ProjectNodes.md`
- `FirstLLM.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 11`。
- `firstllm_cuda_backend_test` 与此前全部测试均通过。
- 总测试时间为 `0.17 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 CUDA backend 只是骨架，不包含 CUDA runtime 检测、CUDA kernel、device memory 或真实 GPU 执行。

设计思考：

- CUDA backend 先作为可选 capability provider 出现在架构中，而不是立即引入 CUDA SDK 依赖。
- 让不可用 backend 明确返回 `BackendUnavailable`，比静默初始化成功更安全。
- 第一版 `supports()` 返回 false，确保当前 CPU-only 构建不会误把请求分配给 CUDA。

下一步：

- 进入第 11 章，创建 GGUF reader。

## 16. 第 11 章：GGUF reader

### 2026-07-06 17:59:26 +08:00

章节 / 阶段：第 11 章 GGUF header reader

完成内容：

- 用户已手动创建 `include/firstllm/model/gguf_reader.h`。
- 用户已手动创建 `src/model/gguf_reader.cpp`。
- 用户已手动创建 `tests/gguf_reader_test.cpp`。
- 用户已将 `src/model/gguf_reader.cpp` 接入 `firstllm` 静态库目标。
- 用户已将 `firstllm_gguf_reader_test` 接入 CMake 和 CTest。
- `GgufReader` 已保存 GGUF 文件路径，并暴露 `path()`、`header()` 和 `read_header()`。
- `read_header()` 已按 little-endian 读取 magic、version、tensor_count 和 metadata_kv_count。
- 测试已覆盖正常 header、空路径、文件不存在、magic 错误和截断文件。
- Agent 检查文件后重新执行 configure、build 和 ctest，确认 GGUF header reader 通过。

新增文件：

- `include/firstllm/model/gguf_reader.h`
- `src/model/gguf_reader.cpp`
- `tests/gguf_reader_test.cpp`

修改文件：

- `CMakeLists.txt`
- `ProgressLog.md`
- `ProjectNodes.md`
- `FirstLLM.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 12`。
- `firstllm_gguf_reader_test` 与此前全部测试均通过。
- 总测试时间为 `0.14 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 GGUF reader 只读取固定 header，不解析 metadata key/value、tensor info 或权重数据。

设计思考：

- GGUF 是二进制格式，当前实现手动按 little-endian 读取整数，避免依赖平台字节序和结构体内存布局。
- 第一版先验证文件识别和错误路径，让后续 metadata 与 tensor info 解析有稳定入口。
- 测试通过临时文件构造最小 GGUF-like header，避免依赖真实大模型文件。

下一步：

- 继续第 11 章，扩展 metadata key/value 读取。

### 2026-07-06 18:18:27 +08:00

章节 / 阶段：第 11 章 GGUF metadata key/value reader

完成内容：

- 根据用户请求，Agent 接手完成 GGUF metadata key/value 读取相关代码修改和测试扩展。
- `GgufReader` 已暴露 `metadata()` 和 `read_metadata()`。
- `GgufMetadataValueType` 已覆盖 GGUF metadata value type 枚举。
- `GgufMetadataValue` 当前支持 `std::uint32_t`、`std::uint64_t`、`bool` 和 `std::string`。
- `read_metadata()` 已能在读取 header 后，逐条读取 metadata key、value type 和 value。
- 当前遇到不支持的 value type 会返回 `InvalidArgument`，避免错误跳读二进制数据。
- bool metadata 会校验原始字节只能是 `0` 或 `1`。
- `tests/gguf_reader_test.cpp` 已扩展为构造带 metadata 的最小 GGUF-like 文件，并验证成功与错误路径。

修改文件：

- `include/firstllm/model/gguf_reader.h`
- `src/model/gguf_reader.cpp`
- `tests/gguf_reader_test.cpp`
- `ProgressLog.md`
- `ProjectNodes.md`
- `FirstLLM.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 12`。
- `firstllm_gguf_reader_test` 与此前全部测试均通过。
- 总测试时间为 `9.43 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用外部执行权限重新运行相同 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 GGUF reader 仍不支持 array metadata、float metadata、int metadata、tensor info 或权重数据。

设计思考：

- 先支持 `uint32`、`uint64`、`bool` 和 `string`，可以覆盖早期最常用的模型基础 metadata。
- 对暂不支持的 metadata type 立即返回错误，比试图跳过未知二进制布局更稳。
- 测试继续使用临时文件手动写 little-endian 数据，不依赖真实模型文件。

下一步：

- 继续第 11 章，扩展 tensor info 读取。

### 2026-07-07 09:59:37 +08:00

章节 / 阶段：第 11 章 GGUF tensor info reader

完成内容：

- 根据当前第 11 章路线，Agent 接手完成 GGUF tensor info 读取。
- 新增 `GgufTensorType`，用于命名常见 GGML tensor type 编号。
- 新增 `GgufTensorInfo`，保存 tensor 名称、维度列表、类型和数据 offset。
- `GgufReader` 已暴露 `tensor_infos()` 和 `read_tensor_infos()`。
- `read_tensor_infos()` 会依次读取 header、metadata 和 tensor info，但不会读取真实权重数据。
- tensor info 读取会校验维度数量不能为 0 或超过当前上限，也会拒绝维度大小为 0 的异常数据。
- `tests/gguf_reader_test.cpp` 已扩展正常 tensor info 文件、非法维度数量和截断 tensor info 三类测试。

修改文件：

- `include/firstllm/model/gguf_reader.h`
- `src/model/gguf_reader.cpp`
- `tests/gguf_reader_test.cpp`
- `CMakeLists.txt`
- `ProgressLog.md`
- `ProjectNodes.md`
- `FirstLLM.md`

验证情况：

- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 12`。
- `firstllm_gguf_reader_test` 与此前全部测试均通过。
- 总测试时间为 `289.55 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会在文件跟踪阶段遇到 `拒绝访问`。
- 使用提升权限重新运行同一 build 命令后构建成功，说明该问题与当前代码无关，更像是构建工具访问权限限制。
- 当前 GGUF reader 仍不支持 array metadata、float metadata、int metadata、data section 对齐计算、真实权重读取或 mmap。

设计思考：

- tensor info 是权重加载前的索引层：它告诉我们有哪些 tensor、每个 tensor 的形状、类型和相对偏移。
- 本阶段只保存结构信息，不读取权重字节，可以保持测试文件很小，也避免过早引入大模型文件依赖。
- 暂时不解释量化 block 布局，只保存 type 编号；真正加载权重时再处理不同 tensor type 的字节布局。

下一步：

- 继续第 11 章，计算 data section 起点与 tensor 数据绝对偏移。

### 2026-07-08 15:33:02 +08:00

章节 / 阶段：第 11 章 GGUF data section 与 tensor 数据偏移

完成内容：

- 根据用户请求，Agent 接手完成 GGUF data section 对齐和 tensor 数据绝对偏移计算。
- `GgufReader` 新增 `data_section_offset()`，用于返回对齐后的 data section 文件绝对起点。
- `GgufTensorInfo` 新增 `data_offset`，用于保存 tensor 数据在整个 GGUF 文件中的绝对偏移。
- `read_tensor_infos()` 现在会在读取 header、metadata 和 tensor info 后，根据 `general.alignment` 计算 data section 起点。
- 当 GGUF 文件没有 `general.alignment` metadata 时，reader 使用默认 32 字节对齐。
- reader 会拒绝 `general.alignment = 0`，也会拒绝 tensor 相对 offset 加上 data section 起点后的 uint64 溢出。
- `tests/gguf_reader_test.cpp` 已扩展默认对齐、自定义 `general.alignment`、非法 alignment 和 tensor offset 溢出测试。

新增文件 / 目录：

- 无。

修改文件：

- `include/firstllm/model/gguf_reader.h`
- `src/model/gguf_reader.cpp`
- `tests/gguf_reader_test.cpp`
- `FirstLLM.md`
- `ProjectNodes.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- Codex 沙箱内直接执行 Debug build 时，MSBuild 在 `ZERO_CHECK.vcxproj` 的 `FileTracker` 阶段触发 `拒绝访问`，与既有环境现象一致。
- 使用提升权限重跑同一条 Debug build 命令后构建成功。
- 单独运行 `build/Debug/firstllm_gguf_reader_test.exe` 成功，输出 `gguf_reader_test passed`。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 12`。
- 总测试时间为 `265.14 sec`。

已知问题 / Bug：

- 当前 GGUF reader 仍不支持 array metadata、float metadata、int metadata、真实权重读取或 mmap。
- 当前只计算 tensor 数据偏移，不校验真实权重字节长度，也不解释量化 block 布局。
- MSBuild `FileTracker` 权限问题仍是当前 Codex 沙箱内的已知环境现象，不是源码错误。

设计思考：

- GGUF tensor info 中的 `offset` 是相对 data section 的偏移，不能直接当作文件绝对位置使用。
- 先把 data section 起点和每个 tensor 的绝对偏移算清楚，可以为后续 ModelLoader 或权重 mmap 留出稳定索引。
- 本阶段继续不读取真实权重，保持 GGUF reader 的职责集中在结构解析和权重定位。

下一步：

- 第 11 章 GGUF reader 首版完成，进入第 12 章 Tiny Llama-like forward 的最小可验证路径。

## 17. 第 12 章：Tiny Llama-like forward

### 2026-07-08 16:10:01 +08:00

章节 / 阶段：第 12 章 Tiny Llama-like forward

完成内容：

- 根据用户请求，Agent 接手完成 Tiny Llama-like forward 首版。
- 新增 `TinyLlamaConfig`，记录 `hidden_size`、`vocab_size` 和 `rms_norm_epsilon`。
- 新增 `TinyLlamaWeights`，以外部 `Tensor` 指针引用 `final_norm_weight`、`output_projection` 和 `output_bias`。
- 新增 `TinyLlamaModel`，在 model 层建立第一条最小 forward 路径。
- `TinyLlamaModel::forward()` 已串联 `CpuRmsNorm`、`CpuMatMul`、`CpuAdd` 和 `CpuSoftmaxLastDim`。
- 当前 forward 输入为 `[tokens, hidden_size]`，输出为 `[tokens, vocab_size]` 的概率分布。
- 因为 `CpuAdd` 暂不支持 broadcast，模型层会先把 `[vocab_size]` bias 展开成 `[tokens, vocab_size]` 再调用 `CpuAdd`。
- `tests/tiny_llama_test.cpp` 使用内存中的小权重和小输入验证确定性数值结果。

新增文件 / 目录：

- `include/firstllm/model/tiny_llama.h`
- `src/model/tiny_llama.cpp`
- `tests/tiny_llama_test.cpp`

修改文件：

- `CMakeLists.txt`
- `FirstLLM.md`
- `ProjectNodes.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- Codex 沙箱内直接执行 Debug build 时，MSBuild 在 `ZERO_CHECK.vcxproj` 的 `FileTracker` 阶段触发 `拒绝访问`，与既有环境现象一致。
- 使用提升权限重跑同一条 Debug build 命令后构建成功。
- 单独运行 `build/Debug/firstllm_tiny_llama_test.exe` 成功，输出 `tiny_llama_test passed`。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 13`。
- 总测试时间为 `9.52 sec`。

已知问题 / Bug：

- 当前 Tiny forward 不是完整 Transformer，不包含 attention、MLP、RoPE、KV cache、tokenizer、sampler 或真实文本生成。
- 当前权重通过外部 `Tensor` 指针传入，不拥有权重内存，也不从 GGUF reader 加载真实权重。
- 当前只支持 float32 host tensor，不支持 half/bfloat16、量化权重、GPU memory 或 batch 调度。
- 当前 `output_bias` 是为了验证 `CpuAdd` 串联而保留的最小 bias；真实 Llama 输出头通常可以没有 bias。

设计思考：

- 第 12 章的重点不是模型完整性，而是让 model 层第一次真实地组织已有 kernel。
- 先把 `RMSNorm -> MatMul -> Add -> Softmax` 串起来，可以验证 Tensor shape、Status 错误传播和 kernel 调用边界。
- 后续 KV cache 和自回归生成可以接在这个 forward 之后，不需要重写前面已经验证过的基础算子。

下一步：

- 进入第 13 章，设计 KV cache 与自回归生成的最小数据结构和单步接口。

## 18. 第 13 章：KV cache 与自回归生成

### 2026-07-08 16:35:40 +08:00

章节 / 阶段：第 13 章 KV cache

完成内容：

- 根据用户请求，Agent 接手继续推进第 13 章，完成 KV cache 首版。
- 新增 `KvCacheConfig`，记录 layer 数、最大 token 数、KV head 数和 head_dim。
- 新增 `KvCache`，为每层预分配 key/value cache，shape 为 `[max_token_count, kv_head_count, head_dim]`。
- `KvCache::append()` 已能向指定 layer 追加一个 token 的 key/value。
- `KvCache::read_key()` 和 `KvCache::read_value()` 已能按 layer/token/head/dim 读取单个 float32 元素。
- `KvCache::token_count()` 已能查询每层已缓存 token 数。
- `KvCache::clear()` 已能清空所有层的 token 计数，底层内存保留并由后续 append 覆盖。
- `tests/kv_cache_test.cpp` 覆盖正常追加、按层计数、读取 key/value、容量满、未写 token、layer/head/dim 越界、空输出指针、错误 dtype、错误 rank、错误 shape、clear 和无效配置。

新增文件 / 目录：

- `include/firstllm/model/kv_cache.h`
- `src/model/kv_cache.cpp`
- `tests/kv_cache_test.cpp`

修改文件：

- `CMakeLists.txt`
- `FirstLLM.md`
- `ProjectNodes.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- Codex 沙箱内直接执行 Debug build 时，MSBuild 在 `ZERO_CHECK.vcxproj` 的 `FileTracker` 阶段触发 `拒绝访问`，与既有环境现象一致。
- 使用提升权限重跑同一条 Debug build 命令后构建成功。
- 单独运行 `build/Debug/firstllm_kv_cache_test.exe` 成功，输出 `kv_cache_test passed`。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 14`。
- 总测试时间为 `12.15 sec`。

已知问题 / Bug：

- 当前 KV cache 只支持 float32 host tensor，不支持 half/bfloat16、量化 KV、GPU memory 或 batch。
- 当前每层 token_count 独立维护，调用方需要按模型层顺序追加；后续可在生成调度层统一管理 step。
- 当前不实现 attention，也不计算 query 对历史 KV 的注意力分数。
- 当前不做分页 KV cache、滑动窗口、prefix cache 或内存复用优化。

设计思考：

- KV cache 的第一版重点是把“自回归状态会随 token 增长”这件事具象化，而不是追求性能。
- 按 layer/token/head/dim 建立索引后，后续 attention kernel 可以明确知道历史 key/value 从哪里读。
- `clear()` 只重置计数而不清零底层内存，符合常见缓存复用思路，也避免把清理和内存分配绑在一起。

下一步：

- 继续第 13 章，在 KV cache 基础上设计自回归单步生成接口。

### 2026-07-08 17:29:58 +08:00

章节 / 阶段：第 13 章 自回归单步生成接口

完成内容：

- 根据用户请求，Agent 接手完成自回归单步生成接口首版。
- 新增 `GeneratorConfig`，记录最大 token 数、eos token id 和是否遇到 eos 后停止。
- 新增 `GenerationState`，保存已生成 token id 序列和 finished 标记。
- 新增 `GreedyNextToken()`，从概率分布最后一行选择最大概率 token；相同概率时保留更小 token id，保证结果稳定。
- 新增 `GenerateOneStep()`，执行 `hidden_state -> TinyLlamaModel::forward() -> GreedyNextToken() -> 更新 GenerationState`。
- `GenerateOneStep()` 当前要求 `hidden_state` shape 为 `[1, hidden_size]`，保持“单步生成”的边界清晰。
- `tests/generator_test.cpp` 覆盖 greedy 正常路径、tie 规则、空输出指针、错误 dtype/rank、单步生成状态推进、max token 限制、eos finished、空 state/next 指针、坏生成配置和错误 hidden_state。
- `include/firstllm/firstllm.h` 已纳入 `runtime/generator.h`，让最小生成接口成为公共 runtime API。

新增文件 / 目录：

- `include/firstllm/runtime/generator.h`
- `src/runtime/generator.cpp`
- `tests/generator_test.cpp`

修改文件：

- `CMakeLists.txt`
- `include/firstllm/firstllm.h`
- `FirstLLM.md`
- `ProjectNodes.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- Codex 沙箱内直接执行 Debug build 时，MSBuild 在 `ZERO_CHECK.vcxproj` 的 `FileTracker` 阶段触发 `拒绝访问`，与既有环境现象一致。
- 使用提升权限重跑同一条 Debug build 命令后构建成功。
- 单独运行 `build/Debug/firstllm_generator_test.exe` 成功，输出 `generator_test passed`。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 15`。
- 总测试时间为 `14.27 sec`。

已知问题 / Bug：

- 当前 generator 只支持 greedy argmax，不支持 temperature、top-k、top-p 或随机采样。
- 当前 `GenerateOneStep()` 使用调用方传入的 hidden state，不负责 token id 到 embedding 的查表。
- 当前不会把 attention key/value 写入 `KvCache`，因为 Tiny forward 首版还没有 attention 计算。
- 当前不是完整生成循环，只完成单步状态推进。

设计思考：

- 第 13 章首版的重点是打通“概率分布变成 token id”的控制流。
- `GreedyNextToken()` 独立出来后，后续 sampler 可以在同一位置替换为 temperature/top-k/top-p。
- `GenerationState` 先只保存 token id 和 finished 标记，避免在还没有 tokenizer/sampler 前过早设计复杂状态。

下一步：

- 进入第 14 章，准备 tokenizer、sampler 和真实文本生成的最小接口。

## 19. 文档与协作流程日志

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

### 2026-07-07 09:33:13 +08:00

章节 / 阶段：中文注释与协作流程更新

完成内容：

- 根据用户要求，为已有核心代码、测试代码和示例代码补充中文注释。
- 注释覆盖函数职责、关键变量含义、错误路径、当前未完善功能和后续可扩展点。
- 更新 `agent.md`，明确当前协作方式：默认由用户手敲代码；当用户明确要求“帮我完成”时，Agent 可以直接修改代码并完成验证。
- 明确后续新增代码示例也需要配套中文注释，方便学习和回看。
- 在 `CMakeLists.txt` 中为 MSVC 添加 `/utf-8` 编译选项，避免中文注释在默认代码页下导致乱码和编译错误。

修改文件：

- `agent.md`
- `CMakeLists.txt`
- `include/firstllm/**`
- `src/**`
- `tests/**`
- `examples/firstllm_info.cpp`
- `ProgressLog.md`

验证情况：

- CMake configure 成功。
- CMake build 成功。
- CTest 运行成功。
- 测试结果为 `100% tests passed, 0 tests failed out of 12`。
- 总测试时间为 `289.59 sec`。

已知问题 / Bug：

- Codex 沙箱内执行 MSBuild 时仍会触发 `Microsoft.Build.Utilities.FileTracker` 权限错误。
- 使用提升权限运行同一构建命令后构建成功，说明该问题来自构建工具权限限制，不是当前源码问题。

设计思考：

- 本项目是学习型推理引擎，中文注释应服务于理解代码意图，而不是机械翻译每一行语句。
- 对暂未实现的功能保留明确注释，可以让后续章节知道该从哪里继续扩展。

下一步：

- 回到第 11 章 GGUF reader，继续实现 tensor info 读取。

### 2026-07-08 15:33:02 +08:00

章节 / 阶段：文档状态同步

完成内容：

- Agent 根据本次 GGUF data section 与 tensor 数据偏移实现结果，同步更新 `FirstLLM.md`、`ProjectNodes.md` 和 `agent.md`。
- 当前文档均指向：第 11 章 GGUF reader 首版已完成，下一步进入第 12 章 Tiny Llama-like forward。

修改文件：

- `FirstLLM.md`
- `ProjectNodes.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- 文档同步与代码验证使用同一次 CTest 结果：`100% tests passed, 0 tests failed out of 12`。

下一步：

- 进入第 12 章 Tiny Llama-like forward 的最小可验证路径。
