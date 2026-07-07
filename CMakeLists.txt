cmake_minimum_required(VERSION 3.20)

project(FirstLLM VERSION 0.1.0 LANGUAGES CXX)

option(FIRSTLLM_BUILD_EXAMPLES "Build FirstLLM examples" ON)
option(FIRSTLLM_BUILD_TESTS "Build FirstLLM tests" ON)

# firstllm 静态库是项目核心产物，按层次列出源码。
add_library(firstllm STATIC
  # Core 基础类型。
  src/core/status.cpp
  src/core/tensor.cpp
  src/core/backend.cpp
  # Backend 实现。CUDA 当前只是无 SDK 依赖的骨架。
  src/backends/cpu/cpu_backend.cpp
  src/backends/cuda/cuda_backend.cpp
  # Runtime 用户侧入口。
  src/runtime/engine.cpp
  # CPU kernel 正确性基线。
  src/kernels/cpu/add.cpp
  src/kernels/cpu/matmul.cpp
  src/kernels/cpu/softmax.cpp
  src/kernels/cpu/rms_norm.cpp
  # Model 层：当前只读取 GGUF header 和部分 metadata。
  src/model/gguf_reader.cpp
)

target_compile_features(firstllm PUBLIC cxx_std_20)

if(MSVC)
  # 中文注释统一按 UTF-8 解析，避免 MSVC 默认代码页导致注释乱码甚至编译报错。
  target_compile_options(firstllm PUBLIC /utf-8)
endif()

target_include_directories(firstllm
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(FIRSTLLM_BUILD_EXAMPLES)
  # 最小可运行示例：初始化 Engine 并打印选中的 backend。
  add_executable(firstllm_info
    examples/firstllm_info.cpp
  )

  target_link_libraries(firstllm_info
    PRIVATE
      firstllm
  )
endif()

if(FIRSTLLM_BUILD_TESTS)
  enable_testing()

  # Core: Status 测试。
  add_executable(firstllm_status_test
    tests/status_test.cpp
  )

  target_link_libraries(firstllm_status_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_status_test COMMAND firstllm_status_test)

  # Core: Tensor 测试。
  add_executable(firstllm_tensor_test
    tests/tensor_test.cpp
  )

  target_link_libraries(firstllm_tensor_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_tensor_test COMMAND firstllm_tensor_test)

  # Core: BackendRegistry 测试。
  add_executable(firstllm_backend_test
    tests/backend_test.cpp
  )

  target_link_libraries(firstllm_backend_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_backend_test COMMAND firstllm_backend_test)

  # Backend: CPU backend 测试。
  add_executable(firstllm_cpu_backend_test
    tests/cpu_backend_test.cpp
  )

  target_link_libraries(firstllm_cpu_backend_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_backend_test COMMAND firstllm_cpu_backend_test)

  # Backend: CUDA 骨架测试。当前只验证“存在但不可用”。
  add_executable(firstllm_cuda_backend_test
    tests/cuda_backend_test.cpp
  )

  target_link_libraries(firstllm_cuda_backend_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cuda_backend_test COMMAND firstllm_cuda_backend_test)

  # Runtime: Engine 初始化和 backend 查找测试。
  add_executable(firstllm_engine_test
    tests/engine_test.cpp
  )

  target_link_libraries(firstllm_engine_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_engine_test COMMAND firstllm_engine_test)

  # Public header: 系统级 smoke test。
  add_executable(firstllm_smoke_test
    tests/smoke.cpp
  )

  target_link_libraries(firstllm_smoke_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_smoke_test COMMAND firstllm_smoke_test)

  # Kernel: CPU add 测试。
  add_executable(firstllm_cpu_add_test
    tests/cpu_add_test.cpp
  )

  target_link_libraries(firstllm_cpu_add_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_add_test COMMAND firstllm_cpu_add_test)

  # Kernel: CPU matmul 测试。
  add_executable(firstllm_cpu_matmul_test
    tests/cpu_matmul_test.cpp
  )

  target_link_libraries(firstllm_cpu_matmul_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_matmul_test COMMAND firstllm_cpu_matmul_test)

  # Kernel: CPU softmax 测试。
  add_executable(firstllm_cpu_softmax_test
    tests/cpu_softmax_test.cpp
  )

  target_link_libraries(firstllm_cpu_softmax_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_softmax_test COMMAND firstllm_cpu_softmax_test)

  # Kernel: CPU RMSNorm 测试。
  add_executable(firstllm_cpu_rms_norm_test
    tests/cpu_rms_norm_test.cpp
  )

  target_link_libraries(firstllm_cpu_rms_norm_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_rms_norm_test COMMAND firstllm_cpu_rms_norm_test)

  # Model: GGUF header 和 metadata reader 测试。
  add_executable(firstllm_gguf_reader_test
    tests/gguf_reader_test.cpp
  )

  target_link_libraries(firstllm_gguf_reader_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_gguf_reader_test COMMAND firstllm_gguf_reader_test)
endif()
