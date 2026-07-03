# FirstLLM 环境搭建指南

## 1. 目标

本文用于搭建 FirstLLM 的本地开发环境。FirstLLM 当前是 C++20 + CMake 项目，第一阶段需要能完成：

- 配置 C++ 编译器。
- 配置 CMake。
- 配置 Git。
- 构建 `firstllm` 静态库。
- 运行 example。
- 运行 smoke test。

后续 CUDA backend 会作为可选能力加入，因此 CUDA 环境不作为当前必需项。

## 2. 当前推荐环境

### 必需工具

| 工具 | 推荐版本 | 用途 |
| --- | --- | --- |
| Git | 2.x | 版本管理和查看项目变更 |
| CMake | 3.20 或更高 | 生成构建工程 |
| MSVC Build Tools | Visual Studio 2022 Build Tools | Windows 上的 C++20 编译器 |
| Windows SDK | 随 MSVC Build Tools 安装 | Windows C++ 编译需要 |

### 可选工具

| 工具 | 用途 |
| --- | --- |
| Ninja | 更轻量的 CMake 构建后端 |
| CUDA Toolkit | 后续实现 CUDA backend |
| Nsight Systems / Nsight Compute | 后续 CUDA 性能分析 |

## 3. Windows 推荐安装方式

如果系统有 `winget`，可以使用以下命令安装工具。

安装 Git：

```powershell
winget install --id Git.Git --exact --source winget --accept-package-agreements --accept-source-agreements
```

安装 CMake：

```powershell
winget install --id Kitware.CMake --exact --source winget --accept-package-agreements --accept-source-agreements
```

安装 Ninja：

```powershell
winget install --id Ninja-build.Ninja --exact --source winget --accept-package-agreements --accept-source-agreements
```

安装 Visual Studio 2022 Build Tools：

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --source winget --accept-package-agreements --accept-source-agreements --override "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --add Microsoft.VisualStudio.Component.VC.CMake.Project"
```

说明：

- MSVC Build Tools 安装体积较大，耗时可能较长。
- 安装后建议重新打开 PowerShell。
- 如果 `cl` 仍然不可见，需要进入 Visual Studio Developer PowerShell，或运行项目提供的环境脚本。

## 4. 项目环境脚本

本项目提供两个脚本：

```text
scripts/check-env.ps1
scripts/firstllm-env.ps1
```

### 4.1 检查环境

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\check-env.ps1
```

它会检查：

- Git
- CMake
- MSVC `cl`
- Visual Studio `vswhere`
- Ninja
- CUDA `nvcc`
- 常见安装路径

### 4.2 配置当前 PowerShell 会话

```powershell
. .\scripts\firstllm-env.ps1
```

注意命令最前面有一个点和一个空格，这表示把脚本加载到当前 PowerShell 会话里，而不是开一个子进程。这样脚本设置的 PATH 才会在当前窗口中生效。

该脚本会尝试：

- 将 Git 常见安装路径加入当前 PATH。
- 将 CMake 常见安装路径加入当前 PATH。
- 将 Ninja 常见安装路径加入当前 PATH。
- 将 CUDA 常见安装路径加入当前 PATH。
- 如果发现 Visual Studio Build Tools，则加载 MSVC 编译环境。

如果 PowerShell 提示脚本执行被禁用，可以先启动一个临时放宽执行策略的 PowerShell：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile
. .\scripts\firstllm-env.ps1
```

也可以只做一次性检查：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; git --version; cmake --version; Get-Command cl"
```

## 5. 构建项目

### 5.1 Visual Studio 生成器

适合 Windows + MSVC：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
ctest --test-dir build -C Debug
```

运行示例：

```powershell
.\build\Debug\firstllm_info.exe
```

不同 CMake 生成器下，示例程序位置可能略有差异。

### 5.2 Ninja 生成器

适合已经加载 MSVC 开发环境的 PowerShell：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```

## 6. 当前仓库的环境状态

### 6.1 初始探测状态

截至 `2026-07-03 14:41:35 +08:00`，初始环境探测结果：

- `winget` 可用。
- Git 已安装在 `C:\Program Files\Git\cmd\git.exe`，但当前 shell 的 PATH 没有识别到 `git`。
- 未在 PATH 中发现 CMake。
- 未在 PATH 中发现 MSVC `cl`。
- 未在常见路径发现 Visual Studio `vswhere.exe`。
- 未在 PATH 中发现 CUDA `nvcc`。
- 未在常见路径发现 CUDA Toolkit。

因此当前必须补齐 CMake 和 C++ 编译器，才能完成真实构建验证。

### 6.2 已完成配置状态

截至 `2026-07-03 15:01:45 +08:00`，已完成以下配置：

- 已通过 `winget` 安装 CMake。
- 已通过 `winget` 安装 Visual Studio 2022 Build Tools 的 C++ 工具链。
- 运行 `. .\scripts\firstllm-env.ps1` 后，可以找到 Git、CMake、MSVC `cl` 和 Ninja。
- 已成功执行 CMake configure。
- 已成功构建 `firstllm.lib`、`firstllm_info.exe` 和 `firstllm_smoke.exe`。
- 已成功运行 `ctest`，`firstllm_smoke` 测试通过。
- 已成功运行 `firstllm_info.exe` 示例。
- CUDA / `nvcc` 仍未安装；当前阶段不强制需要。

验证命令：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; cmake -S . -B build -G 'Visual Studio 17 2022' -A x64"
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; cmake --build build --config Debug"
powershell -ExecutionPolicy Bypass -NoProfile -Command ". .\scripts\firstllm-env.ps1; ctest --test-dir build -C Debug --output-on-failure"
.\build\Debug\firstllm_info.exe
```

## 7. 常见问题

### 7.1 `cmake` 找不到

原因通常是 CMake 未安装，或安装后未加入 PATH。

处理方式：

1. 安装 CMake。
2. 重新打开 PowerShell。
3. 运行 `scripts/firstllm-env.ps1`。
4. 再执行 `cmake --version`。

### 7.2 `cl` 找不到

原因通常是没有安装 Visual Studio Build Tools，或没有加载 MSVC 开发环境。

处理方式：

1. 安装 Visual Studio 2022 Build Tools，并包含 C++ workload。
2. 重新打开 PowerShell。
3. 运行 `. .\scripts\firstllm-env.ps1`。
4. 再执行 `cl` 或 `cmake -S . -B build`。

### 7.3 `git` 已安装但找不到

当前机器已出现这种情况。可以临时运行：

```powershell
. .\scripts\firstllm-env.ps1
git --version
```

脚本会把 `C:\Program Files\Git\cmd` 加到当前 PATH。

### 7.4 是否现在需要 CUDA

不需要。FirstLLM 的路线是 CPU 优先，同时 CUDA 同步设计。当前阶段只要求 C++ 编译环境可用。CUDA 可以等 CPU 算子和接口稳定后再安装。
