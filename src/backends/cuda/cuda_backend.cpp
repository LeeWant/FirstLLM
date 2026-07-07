#include "firstllm/backends/cuda_backend.h"

namespace firstllm {

// CUDA backend 先占住架构位置，优先级高于 CPU，但当前不可用。
CudaBackend::CudaBackend()
    : info_{"cuda", Device{DeviceType::kCuda, 0}, 10},
      initialized_(false) {}

// 返回 CUDA backend 的静态信息。
BackendInfo CudaBackend::info() const {
  return info_;
}

// 当前版本不检测 CUDA runtime，也不链接 CUDA SDK，因此明确返回不可用。
Status CudaBackend::initialize() {
  initialized_ = false;
  return Status::BackendUnavailable(
      "CUDA backend is not available in this build");
}

// 当前始终不可用，后续接入 CUDA runtime 后再改成真实状态。
bool CudaBackend::is_available() const {
  return initialized_;
}

// 当前没有 CUDA kernel，所有能力查询都返回 false。
bool CudaBackend::supports(OpKind op, DType dtype) const {
  // op/dtype 暂未使用，显式转 void 避免编译器警告。
  (void)op;
  (void)dtype;
  return false;
}

}  // namespace firstllm
