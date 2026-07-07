#include "firstllm/firstllm.h"

#include <iostream>

int main() {
  // engine 是示例程序中的最小运行时入口。
  firstllm::Engine engine(firstllm::EngineConfig{});

  // status 保存初始化结果，失败时打印错误信息。
  const firstllm::Status status = engine.initialize();

  if (!status.ok()) {
    std::cout << "FirstLLM initialize failed: "
              << status.message() << "\n";
    return 1;
  }

  // backend 是当前可用于 float32 add 的后端。
  const auto backend =
      engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

  if (!backend) {
    std::cout << "FirstLLM initialized, but no backend found\n";
    return 1;
  }

  // info 用于展示被选中的 backend。
  const firstllm::BackendInfo info = backend->info();

  std::cout << "FirstLLM initialized\n";
  std::cout << "Selected backend: " << info.name << "\n";

  return 0;
}
