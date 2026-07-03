cmake_minimum_required(VERSION 3.20)

project(FirstLLM VERSION 0.1.0 LANGUAGES CXX)

option(FIRSTLLM_BUILD_EXAMPLES "Build FirstLLM examples" ON)
option(FIRSTLLM_BUILD_TESTS "Build FirstLLM tests" ON)

add_library(firstllm STATIC
  src/core/status.cpp
)

target_compile_features(firstllm PUBLIC cxx_std_20)

target_include_directories(firstllm
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
