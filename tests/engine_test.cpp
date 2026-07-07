#include "firstllm/runtime/engine.h"

#include <cassert>
#include <iostream>

int main() {
  // engine 使用默认配置，应注册 CPU backend。
  firstllm::Engine engine(firstllm::EngineConfig{});

  // status 验证 Engine 初始化是否成功。
  const firstllm::Status status = engine.initialize();

  assert(status.ok());

  // backend 应能找到支持 float32 add 的 CPU backend。
  const auto backend =
      engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

  assert(backend != nullptr);
  assert(backend->info().name == "cpu");
  assert(backend->info().device.type == firstllm::DeviceType::kCpu);

  // missing_dtype 验证 dtype 不支持时不会返回 backend。
  const auto missing_dtype =
      engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kInt32);

  assert(missing_dtype == nullptr);

  // missing_op 验证未声明支持的 op 不会返回 backend。
  const auto missing_op =
      engine.find_backend(firstllm::OpKind::kMatMul, firstllm::DType::kFloat32);

  assert(missing_op == nullptr);

  // disabled_config 关闭 CPU backend，用来验证配置确实生效。
  firstllm::EngineConfig disabled_config;
  disabled_config.enable_cpu_backend = false;

  // cpu_disabled 是不注册 CPU backend 的 Engine。
  firstllm::Engine cpu_disabled(disabled_config);

  const firstllm::Status disabled_status = cpu_disabled.initialize();

  assert(disabled_status.ok());

  // disabled_backend 应为空，因为没有任何 backend 被注册。
  const auto disabled_backend =
      cpu_disabled.find_backend(firstllm::OpKind::kAdd,
                                firstllm::DType::kFloat32);

  assert(disabled_backend == nullptr);

  std::cout << "engine_test passed\n";
  return 0;
}
