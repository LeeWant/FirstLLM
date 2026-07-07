#include "firstllm/firstllm.h"

#include <cassert>
#include <iostream>

int main() {
  // engine 验证 public header 能把 runtime/backend/core 串起来。
  firstllm::Engine engine(firstllm::EngineConfig{});

  // status 验证 Engine 初始化路径成功。
  const firstllm::Status status = engine.initialize();

  assert(status.ok());

  // backend 验证通过总入口可以找到 CPU backend。
  const auto backend =
      engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

  assert(backend != nullptr);
  assert(backend->info().name == "cpu");

  // tensor 验证 public header 也暴露了 Tensor 基础类型。
  firstllm::Tensor tensor(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 2}));

  assert(tensor.num_elements() == 4);
  assert(tensor.byte_size() == 16);

  std::cout << "smoke_test passed\n";
  return 0;
}
