#pragma once

#include "firstllm/core/backend.h"

namespace firstllm {

// CUDA backend 当前只是骨架：保留架构位置，但不依赖 CUDA SDK。
class CudaBackend : public Backend {
 public:
  // 构造 CUDA backend 信息；当前 build 下 initialize() 会明确返回不可用。
  CudaBackend();

  // 返回 CUDA backend 的名称、设备和优先级。
  BackendInfo info() const override;

  // 初始化 CUDA backend；当前版本故意返回 BackendUnavailable。
  Status initialize() override;

  // 返回是否可用；当前始终为 false。
  bool is_available() const override;

  // 查询 CUDA 支持能力；当前无真实 CUDA kernel，因此全部返回 false。
  bool supports(OpKind op, DType dtype) const override;

 private:
  BackendInfo info_;  // backend 描述信息，优先级高于 CPU，但当前不可用。
  bool initialized_;  // 当前始终保持 false，后续接入 CUDA runtime 后再扩展。
};

}  // namespace firstllm
