#include "firstllm/backends/cpu_backend.h"

namespace firstllm {

// CPU backend 初始状态不可用，initialize() 成功后才可被 registry 选择。
CpuBackend::CpuBackend()
    : info_{"cpu", Device{DeviceType::kCpu, 0}, 0},
      initialized_(false) {}

// 返回 backend 信息，拷贝返回保持接口简单。
BackendInfo CpuBackend::info() const {
  return info_;
}

// CPU backend 当前没有额外资源要申请，因此初始化直接成功。
Status CpuBackend::initialize() {
  initialized_ = true;
  return Status::Ok();
}

// registry 会通过这个函数判断 backend 是否可参与调度。
bool CpuBackend::is_available() const {
  return initialized_;
}

// 当前只声明 float32 add。虽然其它 CPU kernel 已存在，registry 能力表还未接入它们。
bool CpuBackend::supports(OpKind op, DType dtype) const {
  if (dtype != DType::kFloat32) {
    return false;
  }

  return op == OpKind::kAdd;
}

}  // namespace firstllm
