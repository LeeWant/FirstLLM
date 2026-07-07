#include "firstllm/backends/cuda_backend.h"

#include <cassert>
#include <iostream>

int main() {
  // backend 是 CUDA backend 骨架，当前构建下不会真正可用。
  firstllm::CudaBackend backend;

  // info 验证 CUDA backend 已经作为架构节点存在。
  const firstllm::BackendInfo info = backend.info();

  assert(info.name == "cuda");
  assert(info.device.type == firstllm::DeviceType::kCuda);
  assert(info.device.index == 0);
  assert(info.priority == 10);

  assert(!backend.is_available());

  // status 当前应为 BackendUnavailable，说明没有静默假装 CUDA 可用。
  const firstllm::Status status = backend.initialize();

  assert(!status.ok());
  assert(status.code() == firstllm::ErrorCode::kBackendUnavailable);
  assert(!backend.is_available());

  // 当前没有 CUDA kernel，因此所有能力查询都应返回 false。
  assert(!backend.supports(firstllm::OpKind::kAdd,
                           firstllm::DType::kFloat32));

  assert(!backend.supports(firstllm::OpKind::kMatMul,
                           firstllm::DType::kFloat32));

  std::cout << "cuda_backend_test passed\n";
  return 0;
}
