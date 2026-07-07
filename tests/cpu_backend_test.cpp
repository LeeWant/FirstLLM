#include "firstllm/backends/cpu_backend.h"

#include <cassert>
#include <iostream>

int main() {
  // backend 是被测 CPU backend 实例，构造后尚未初始化。
  firstllm::CpuBackend backend;

  // info 是 backend 的静态描述信息。
  const firstllm::BackendInfo info = backend.info();

  assert(info.name == "cpu");
  assert(info.device.type == firstllm::DeviceType::kCpu);
  assert(info.device.index == 0);
  assert(info.priority == 0);

  assert(!backend.is_available());

  // status 验证 CPU backend 初始化是否成功。
  const firstllm::Status status = backend.initialize();

  assert(status.ok());
  assert(backend.is_available());

  // 当前 CPU backend 只声明支持 float32 add。
  assert(backend.supports(firstllm::OpKind::kAdd,
                          firstllm::DType::kFloat32));

  assert(!backend.supports(firstllm::OpKind::kAdd,
                           firstllm::DType::kInt32));

  assert(!backend.supports(firstllm::OpKind::kMatMul,
                           firstllm::DType::kFloat32));

  std::cout << "cpu_backend_test passed\n";
  return 0;
}
