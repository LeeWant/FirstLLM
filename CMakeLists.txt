cmake_minimum_required(VERSION 3.20)

project(FirstLLM VERSION 0.1.0 LANGUAGES CXX)

option(FIRSTLLM_BUILD_EXAMPLES "Build FirstLLM examples" ON)
option(FIRSTLLM_BUILD_TESTS "Build FirstLLM tests" ON)

add_library(firstllm STATIC
  src/core/status.cpp
  src/core/tensor.cpp
  src/core/backend.cpp
  src/backends/cpu/cpu_backend.cpp
  src/runtime/engine.cpp
  src/kernels/cpu/add.cpp
  src/kernels/cpu/matmul.cpp
)

target_compile_features(firstllm PUBLIC cxx_std_20)

target_include_directories(firstllm
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(FIRSTLLM_BUILD_EXAMPLES)
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

  add_executable(firstllm_status_test
    tests/status_test.cpp
  )

  target_link_libraries(firstllm_status_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_status_test COMMAND firstllm_status_test)

  add_executable(firstllm_tensor_test
    tests/tensor_test.cpp
  )

  target_link_libraries(firstllm_tensor_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_tensor_test COMMAND firstllm_tensor_test)

  add_executable(firstllm_backend_test
    tests/backend_test.cpp
  )

  target_link_libraries(firstllm_backend_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_backend_test COMMAND firstllm_backend_test)

  add_executable(firstllm_cpu_backend_test
    tests/cpu_backend_test.cpp
  )

  target_link_libraries(firstllm_cpu_backend_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_backend_test COMMAND firstllm_cpu_backend_test)

  add_executable(firstllm_engine_test
    tests/engine_test.cpp
  )

  target_link_libraries(firstllm_engine_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_engine_test COMMAND firstllm_engine_test)

  add_executable(firstllm_smoke_test
    tests/smoke.cpp
  )

  target_link_libraries(firstllm_smoke_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_smoke_test COMMAND firstllm_smoke_test)

  add_executable(firstllm_cpu_add_test
    tests/cpu_add_test.cpp
  )

  target_link_libraries(firstllm_cpu_add_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_add_test COMMAND firstllm_cpu_add_test)

  add_executable(firstllm_cpu_matmul_test
    tests/cpu_matmul_test.cpp
  )

  target_link_libraries(firstllm_cpu_matmul_test
    PRIVATE
      firstllm
  )

  add_test(NAME firstllm_cpu_matmul_test COMMAND firstllm_cpu_matmul_test)
endif()