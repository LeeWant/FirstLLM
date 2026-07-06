#include "firstllm/backends/cpu_backend.h"

namespace firstllm {

CpuBackend::CpuBackend()
    : info_{"cpu", Device{DeviceType::kCpu, 0}, 0},
      initialized_(false) {}

BackendInfo CpuBackend::info() const {
  return info_;
}

Status CpuBackend::initialize() {
  initialized_ = true;
  return Status::Ok();
}

bool CpuBackend::is_available() const {
  return initialized_;
}

bool CpuBackend::supports(OpKind op, DType dtype) const {
  if (dtype != DType::kFloat32) {
    return false;
  }

  return op == OpKind::kAdd;
}

}  // namespace firstllm