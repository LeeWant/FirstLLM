#include "firstllm/backends/cuda_backend.h"

namespace firstllm {

CudaBackend::CudaBackend()
    : info_{"cuda", Device{DeviceType::kCuda, 0}, 10},
      initialized_(false) {}

BackendInfo CudaBackend::info() const {
  return info_;
}

Status CudaBackend::initialize() {
  initialized_ = false;
  return Status::BackendUnavailable(
      "CUDA backend is not available in this build");
}

bool CudaBackend::is_available() const {
  return initialized_;
}

bool CudaBackend::supports(OpKind op, DType dtype) const {
  (void)op;
  (void)dtype;
  return false;
}

}  // namespace firstllm