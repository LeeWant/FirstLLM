#pragma once

#include "firstllm/core/backend.h"
#include "firstllm/core/status.h"

#include <memory>

namespace firstllm {

// Engine 的初始化配置。当前只控制是否注册 CPU backend。
struct EngineConfig {
  bool enable_cpu_backend = true;  // 是否启用 CPU backend；默认启用作为正确性基线。
};

// Engine 是用户侧运行时入口，负责初始化 backend registry 和选择 backend。
class Engine {
 public:
  // 保存配置但不立即初始化，调用方需要显式调用 initialize()。
  explicit Engine(EngineConfig config);

  // 初始化运行时资源；当前只会按配置注册 CPU backend。
  Status initialize();

  // 根据算子和 dtype 查找可用 backend；找不到时返回 nullptr。
  std::shared_ptr<Backend> find_backend(OpKind op, DType dtype) const;

 private:
  EngineConfig config_;              // 运行时配置，决定启用哪些 backend。
  BackendRegistry backend_registry_;  // backend 注册表，供后续调度查询。
};

}  // namespace firstllm
