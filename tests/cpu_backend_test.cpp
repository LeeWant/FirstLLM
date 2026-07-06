#include "firstllm/backends/cpu_backend.h"

#include <cassert>
#include <iostream>

int main() {
  firstllm::CpuBackend backend;

  const firstllm::BackendInfo info = backend.info();

  assert(info.name == "cpu");
  assert(info.device.type == firstllm::DeviceType::kCpu);
  assert(info.device.index == 0);
  assert(info.priority == 0);

  assert(!backend.is_available());

  const firstllm::Status status = backend.initialize();

  assert(status.ok());
  assert(backend.is_available());

  assert(backend.supports(firstllm::OpKind::kAdd,
                          firstllm::DType::kFloat32));

  assert(!backend.supports(firstllm::OpKind::kAdd,
                           firstllm::DType::kInt32));

  assert(!backend.supports(firstllm::OpKind::kMatMul,
                           firstllm::DType::kFloat32));

  std::cout << "cpu_backend_test passed\n";
  return 0;
}