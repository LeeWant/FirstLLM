#include "firstllm/backends/cuda_backend.h"

#include <cassert>
#include <iostream>

int main() {
  firstllm::CudaBackend backend;

  const firstllm::BackendInfo info = backend.info();

  assert(info.name == "cuda");
  assert(info.device.type == firstllm::DeviceType::kCuda);
  assert(info.device.index == 0);
  assert(info.priority == 10);

  assert(!backend.is_available());

  const firstllm::Status status = backend.initialize();

  assert(!status.ok());
  assert(status.code() == firstllm::ErrorCode::kBackendUnavailable);
  assert(!backend.is_available());

  assert(!backend.supports(firstllm::OpKind::kAdd,
                           firstllm::DType::kFloat32));

  assert(!backend.supports(firstllm::OpKind::kMatMul,
                           firstllm::DType::kFloat32));

  std::cout << "cuda_backend_test passed\n";
  return 0;
}