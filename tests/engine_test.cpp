#include "firstllm/runtime/engine.h"

#include <cassert>
#include <iostream>

int main() {
  firstllm::Engine engine(firstllm::EngineConfig{});

  const firstllm::Status status = engine.initialize();

  assert(status.ok());

  const auto backend =
      engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

  assert(backend != nullptr);
  assert(backend->info().name == "cpu");
  assert(backend->info().device.type == firstllm::DeviceType::kCpu);

  const auto missing_dtype =
      engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kInt32);

  assert(missing_dtype == nullptr);

  const auto missing_op =
      engine.find_backend(firstllm::OpKind::kMatMul, firstllm::DType::kFloat32);

  assert(missing_op == nullptr);

  firstllm::EngineConfig disabled_config;
  disabled_config.enable_cpu_backend = false;

  firstllm::Engine cpu_disabled(disabled_config);

  const firstllm::Status disabled_status = cpu_disabled.initialize();

  assert(disabled_status.ok());

  const auto disabled_backend =
      cpu_disabled.find_backend(firstllm::OpKind::kAdd,
                                firstllm::DType::kFloat32);

  assert(disabled_backend == nullptr);

  std::cout << "engine_test passed\n";
  return 0;
}