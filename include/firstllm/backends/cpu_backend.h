#pragma once

#include "firstllm/core/backend.h"

namespace firstllm {

// CPU backend 是 correctness baseline：它应当永远可构建、少依赖、便于对照。
class CpuBackend : public Backend {
 public:
  // 构造 CPU backend 信息；此时尚未 initialize，所以还不可用。
  CpuBackend();

  // 返回 CPU backend 的名称、设备和优先级。
  BackendInfo info() const override;

  // 初始化 CPU backend；当前没有外部资源，因此总是成功。
  Status initialize() override;

  // 返回是否已经初始化。
  bool is_available() const override;

  // 查询 CPU backend 支持能力；当前只声明支持 float32 add。
  bool supports(OpKind op, DType dtype) const override;

 private:
  BackendInfo info_;  // backend 描述信息，构造后保持不变。
  bool initialized_;  // 是否已初始化成功。
};

}  // namespace firstllm
