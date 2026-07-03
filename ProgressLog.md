# FirstLLM 进度管理日志

## 1. 文件用途

本文件用于记录 FirstLLM 项目的推进过程，类似项目日志和阶段复盘。它不替代 `FirstLLM.md` 和 `ProjectNodes.md`：

- `FirstLLM.md` 记录项目定位、总体架构和长期设计决策。
- `ProjectNodes.md` 记录项目章节规划、节点顺序和每个模块的学习目标。
- `ProgressLog.md` 记录实际发生了什么、什么时候完成、遇到了什么问题、下一步要做什么。

建议每次完成一个小任务后追加一条日志，保持项目过程可追溯。

## 2. 日志记录模板

```text
时间：
阶段：
完成内容：
新增文件：
修改文件：
验证情况：
已知问题 / Bug：
设计思考：
下一步：
```

## 3. 项目日志

### 2026-07-03 14:32:57 +08:00

阶段：M0 工程骨架与项目方向确认

完成内容：

- 明确 FirstLLM 是一个基于 C++ 的极简大模型推理引擎学习项目。
- 确认项目优先支持 decoder-only 文本生成模型，后续预留多模态扩展空间。
- 确认硬件路线为 CPU 优先，同时 CUDA 同步设计和逐步实现。
- 确认项目更偏学习型代码，强调清晰性、文档、注释和阶段性测试。
- 确认模型格式直接兼容 GGUF，不先设计自定义权重格式。
- 参考 ONNX Runtime、TensorRT-LLM、vLLM、llama.cpp 等推理系统的公开架构特点，整理了 FirstLLM 的初始设计方向。
- 创建项目主设计文档，记录项目定位、架构分层、backend 抽象、路线图和编码原则。
- 创建 C++20 + CMake 的最小工程骨架。
- 创建最小 runtime、backend、tensor、status 等核心接口。
- 创建 CPU backend 作为第一块 baseline backend。
- 创建示例程序和 smoke test。
- 创建项目节点规划文档，按章节规划后续功能实现顺序。

新增文件：

- `FirstLLM.md`
- `ProjectNodes.md`
- `ProgressLog.md`
- `CMakeLists.txt`
- `.gitignore`
- `include/firstllm/firstllm.h`
- `include/firstllm/core/status.h`
- `include/firstllm/core/tensor.h`
- `include/firstllm/core/backend.h`
- `include/firstllm/runtime/engine.h`
- `include/firstllm/backends/cpu_backend.h`
- `src/core/status.cpp`
- `src/core/tensor.cpp`
- `src/core/backend.cpp`
- `src/runtime/engine.cpp`
- `src/backends/cpu/cpu_backend.cpp`
- `examples/firstllm_info.cpp`
- `tests/smoke.cpp`

修改文件：

- `FirstLLM.md`：将第 8 节从“待确认问题”改为“已确认的项目方向”，并指向 `ProjectNodes.md`。

验证情况：

- 已检查项目文件清单，核心文件已生成在预期目录。
- 当前环境未找到 CMake，因此无法执行 CMake configure/build。
- 当前环境未找到常见 C++ 编译器，因此无法执行真实编译。
- 当前环境未找到 Git，因此无法使用 Git 查看 diff 或状态。
- 已做一轮静态阅读式检查，并补充 `src/core/tensor.cpp` 中 `std::move` 所需头文件。

已知问题 / Bug：

- 当前机器环境缺少 CMake、C++ 编译器和 Git，暂时无法完成真实构建验证。
- PowerShell 终端显示中文时出现乱码，但这更像是控制台编码显示问题；后续如发现文件本身编码异常，再统一处理。
- 当前 `Engine::Generate()` 只是占位实现，还没有 tokenizer、模型加载、forward、sampler 或 KV cache。
- CPU backend 目前只声明 capability，还没有真正执行 `add`、`matmul`、`softmax`、`rms_norm` 等算子。
- CUDA backend 还没有创建，只在设计文档中规划了路线。
- GGUF 兼容还没有实现，当前只确定了直接支持 GGUF 的方向。

设计思考：

- FirstLLM 不应该一开始就追求完整运行真实大模型。更适合先把 runtime、backend、tensor、status、测试和文档地基打稳。
- CPU backend 是 correctness baseline，后续 CUDA backend 的每个算子都应该和 CPU 结果对照。
- backend 不应该直接散落在 runtime 中，而是通过统一接口注册、查询 capability、初始化和选择。
- GGUF 解析应该放在 model layer，不应该让 `Engine` 直接理解二进制模型文件。
- 学习型项目要把“为什么需要这个模块”写清楚，否则很容易只堆代码却没有形成推理引擎架构认知。

下一步：

1. 进入 M1：为当前核心头文件补充学习型注释，解释每个类型的职责和边界。
2. 新增 `docs/Glossary.md`，记录 tensor、backend、kernel、runtime、GGUF、KV cache、sampler 等术语。
3. 考虑将生成相关结构从 `engine.h` 拆到 `include/firstllm/runtime/generation.h`，为后续扩展 request、streaming、多模态输入做准备。
4. 进入 M2 前，先实现 CPU `add`，建立第一个真实 kernel 的接口、实现和测试范式。

### 2026-07-03 14:38:17 +08:00

阶段：M0 工程骨架与协作规范补充

完成内容：

- 新增 `agent.md`，作为 FirstLLM 项目的智能体工作指南。
- 将项目定位、架构原则、文件结构、文件职责、编码规则、进度管理方式和下一步任务写入 `agent.md`。
- 明确未来协作者进入项目后的文档阅读顺序：`FirstLLM.md`、`ProjectNodes.md`、`ProgressLog.md`、`agent.md`、相关代码。
- 明确每完成一个明确任务后，应更新 `ProgressLog.md`，记录完成内容、验证情况、已知问题和下一步。

新增文件：

- `agent.md`

修改文件：

- `ProgressLog.md`

验证情况：

- 已创建 `agent.md`。
- 已将本次新增工作追加到 `ProgressLog.md`。
- 当前环境仍未执行真实 CMake 构建验证。

已知问题 / Bug：

- `agent.md` 是协作规范文档，不验证代码正确性。
- 真实构建验证仍依赖后续安装 CMake 和 C++ 编译器。

设计思考：

- `agent.md` 的作用是减少未来协作者进入项目时的认知成本。
- `FirstLLM.md` 负责回答“项目是什么”，`ProjectNodes.md` 负责回答“项目怎么推进”，`ProgressLog.md` 负责回答“已经发生了什么”，`agent.md` 负责回答“协作者应该如何工作”。

下一步：

1. 进入 M1，为核心头文件补充学习型注释。
2. 新增 `docs/Glossary.md`。
3. 逐步将 runtime 生成相关结构拆分到独立文件。

### 2026-07-03 14:41:35 +08:00

阶段：M0 环境搭建准备

完成内容：

- 开始整理 FirstLLM 的开发环境搭建流程。
- 探测当前机器上的基础工具链状态。
- 新增 `EnvironmentSetup.md`，记录 Windows 下推荐的 Git、CMake、MSVC Build Tools、Ninja 和 CUDA 安装方式。
- 新增 `scripts/check-env.ps1`，用于检查当前 shell 能否找到常用开发工具。
- 新增 `scripts/firstllm-env.ps1`，用于给当前 PowerShell 会话补充常见 PATH，并在发现 Visual Studio Build Tools 时加载 MSVC 开发环境。

新增文件：

- `EnvironmentSetup.md`
- `scripts/check-env.ps1`
- `scripts/firstllm-env.ps1`

修改文件：

- `ProgressLog.md`

验证情况：

- 当前 PATH 未发现 `cmake`。
- 当前 PATH 未发现 `git`，但发现 Git 已安装在 `C:\Program Files\Git\cmd\git.exe`。
- 当前 PATH 未发现 MSVC `cl`。
- 常见路径未发现 Visual Studio `vswhere.exe`。
- 当前 PATH 未发现 CUDA `nvcc`。
- 常见路径未发现 CUDA Toolkit。
- 当前系统发现 `winget` 可用。

已知问题 / Bug：

- 当前仍缺少 CMake 和 C++ 编译器，无法构建 FirstLLM。
- Git 已安装但未进入当前 shell 的 PATH。
- `winget list` 在未接受源协议时会等待交互输入，非交互执行会失败；后续安装命令需要显式添加协议接受参数。

设计思考：

- 环境配置也应该纳入项目文档和进度日志，否则后续无法复现“为什么此时不能构建”。
- 对学习型 C++ 项目来说，环境脚本比一次性的手工配置更重要，因为它能帮助用户理解工具链由哪些部分组成。
- CUDA 当前不是必需项，应先完成 CPU 构建环境。

下一步：

1. 使用 `winget` 安装或补齐 CMake。
2. 使用 `winget` 安装 Visual Studio 2022 Build Tools 的 C++ workload。
3. 运行 `scripts/firstllm-env.ps1` 和 `scripts/check-env.ps1`。
4. 尝试 CMake configure/build/test。

### 2026-07-03 14:41:35 +08:00 后续环境安装记录

阶段：M0 环境安装执行

完成内容：

- 通过 `winget` 安装 CMake。
- 通过 `winget` 安装 Visual Studio 2022 Build Tools，并请求安装 C++ workload。
- 修正 `scripts/firstllm-env.ps1` 中 CUDA 版本目录处理方式，使其在未来只有一个 CUDA 版本目录时也更稳。
- 更新 `EnvironmentSetup.md`，补充 PowerShell 执行策略限制下的脚本运行方式。

新增文件：

- 无

修改文件：

- `EnvironmentSetup.md`
- `scripts/firstllm-env.ps1`
- `ProgressLog.md`

验证情况：

- CMake 安装命令已返回 `Successfully installed`。
- Visual Studio Build Tools 安装命令已返回 `Successfully installed`。
- 当前 shell 仍需刷新 PATH 或运行 `scripts/firstllm-env.ps1` 后再验证工具链。

已知问题 / Bug：

- PowerShell 默认执行策略阻止直接 dot-source 项目脚本，需要使用 `-ExecutionPolicy Bypass` 或调整用户执行策略。
- 安装完成后当前 shell 不一定自动获得新的 PATH。

设计思考：

- 对 Windows C++ 项目来说，安装工具和“让当前 shell 找到工具”是两个不同步骤，文档中需要明确区分。
- 由于 FirstLLM 目前还处于 CPU 构建阶段，CUDA 暂时保持可选，不在本次安装中强制配置。

下一步：

1. 运行项目环境脚本，确认 Git、CMake、MSVC 可用。
2. 尝试 CMake configure。
3. 尝试构建并运行 smoke test。

### 2026-07-03 15:01:45 +08:00

阶段：M0 环境搭建完成与构建验证

完成内容：

- 确认 `scripts/firstllm-env.ps1` 可以在 PowerShell `-ExecutionPolicy Bypass` 模式下加载。
- 确认环境脚本可以把 Git 和 CMake 常见路径加入当前 PATH。
- 确认环境脚本可以通过 `vswhere` 加载 Visual Studio Build Tools 的 MSVC 开发环境。
- 确认 Git、CMake、MSVC `cl` 和 Ninja 均可被当前开发会话识别。
- 成功执行 CMake configure。
- 成功执行 CMake build。
- 成功执行 `ctest`。
- 成功运行 `firstllm_info.exe` 示例程序。
- 更新 `EnvironmentSetup.md`，记录已完成配置状态和验证命令。
- 更新 `agent.md` 中过时的环境限制说明，改为当前已完成的环境状态。

新增文件：

- 无

修改文件：

- `EnvironmentSetup.md`
- `agent.md`
- `ProgressLog.md`

验证情况：

- `git --version` 输出 `git version 2.55.0.windows.1`。
- `cmake --version` 输出 `cmake version 4.3.4`。
- `cl` 已被识别，路径位于 Visual Studio 2022 Build Tools 的 MSVC 目录。
- `scripts/check-env.ps1` 在加载环境脚本后显示 Git、CMake、cl、Ninja、winget 可用。
- CMake configure 成功，识别 MSVC 19.44.35228.0 和 Windows SDK 10.0.26100.0。
- CMake build 成功生成 `build\Debug\firstllm.lib`、`build\Debug\firstllm_info.exe` 和 `build\Debug\firstllm_smoke.exe`。
- `ctest --test-dir build -C Debug --output-on-failure` 通过，1 个测试全部通过。
- `build\Debug\firstllm_info.exe` 成功输出已注册的 CPU backend。

已知问题 / Bug：

- CUDA / `nvcc` 仍未安装；当前阶段不影响 CPU 构建和测试。
- 当前普通 PowerShell 仍可能因为执行策略阻止直接运行脚本，需要使用 `powershell -ExecutionPolicy Bypass -NoProfile` 或在受信任环境中调整执行策略。
- 安装工具后，新的 PATH 可能需要重新打开终端或运行 `scripts/firstllm-env.ps1` 才能生效。
- Git 工具已可用，但当前 `D:\workplace\FirstLLM` 目录尚未初始化为 Git 仓库，因此 `git status` 会提示 not a git repository。

设计思考：

- 环境搭建已经从“不可构建”推进到“可配置、可编译、可测试”，FirstLLM 可以正式进入 M1。
- 项目环境脚本的价值已经验证：它避免依赖全局 PATH 是否立即刷新，也让 MSVC 开发环境加载过程可复用。
- CUDA 不应在当前阶段强行安装，先保持 CPU baseline 和接口设计清晰更符合学习型项目路线。

下一步：

1. 进入 M1，为核心头文件补充学习型注释。
2. 新增 `docs/Glossary.md`。
3. 将生成相关结构从 `engine.h` 拆到 `include/firstllm/runtime/generation.h`。
4. 开始为 CPU `add` kernel 设计接口、实现和测试。

### 2026-07-03 15:09:16 +08:00

阶段：M0 切换到从零手敲学习模式

完成内容：

- 根据用户要求，清理此前由智能体生成的 C++ 代码骨架和构建产物。
- 删除 `include/`、`src/`、`examples/`、`tests/`、`build/` 和 `CMakeLists.txt`。
- 保留项目文档、环境指南和环境脚本，作为后续手动实现的学习路线和工具支持。
- 重写 `ProjectNodes.md`，改为从第一个文件开始的手敲学习路线。
- 更新 `agent.md` 的当前文件结构和下一步建议，使其匹配“从零手敲学习模式”。

新增文件：

- 无

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

- 已确认当前可见文件只剩项目文档和环境脚本。
- 删除前已验证所有删除目标都位于 `D:\workplace\FirstLLM` 工作区内。
- 当前项目暂时不能构建，这是预期状态，因为 `CMakeLists.txt` 和源码将由用户按路线手动重建。

已知问题 / Bug：

- `FirstLLM.md` 中仍保留最初设计时的目录结构说明，可作为目标结构参考，不代表当前仓库已经存在这些代码文件。
- 当前目录仍不是 Git 仓库。

设计思考：

- 对学习型项目来说，亲手敲每个文件比直接拥有完整代码更重要。
- 保留环境脚本可以避免重复处理工具链问题，让注意力集中在 C++ 推理引擎本身。
- `ProjectNodes.md` 现在承担“手敲路线图”的角色，每章都应该足够小，让用户能理解后再继续。

下一步：

1. 用户按 `ProjectNodes.md` 第 0 章创建 `CMakeLists.txt`。
2. 用户创建基础目录结构。
3. 用户开始手敲 `Status` 模块。
4. 每完成一步后，把理解、问题和验证结果写入 `ProgressLog.md`。
