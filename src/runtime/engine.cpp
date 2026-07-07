#include "firstllm/runtime/engine.h"

#include "firstllm/backends/cpu_backend.h"

#include <memory>
#include <utility>

namespace firstllm {

// Engine 保存配置，backend registry 初始为空。
Engine::Engine(EngineConfig config)
    : config_(std::move(config)), backend_registry_() {}

// 初始化运行时。当前只注册 CPU backend，后续可按配置加入 CUDA/backend 列表。
Status Engine::initialize() {
  if (config_.enable_cpu_backend) {
    // cpu_backend 是当前正确性基线 backend。
    auto cpu_backend = std::make_shared<CpuBackend>();

    // status 保存 backend 初始化结果，失败时直接向上返回。
    Status status = cpu_backend->initialize();
    if (!status.ok()) {
      return status;
    }

    // 初始化成功后再注册，避免 registry 中出现不可用 CPU backend。
    backend_registry_.add_backend(std::move(cpu_backend));
  }

  return Status::Ok();
}

// 把 backend 选择委托给 BackendRegistry。
std::shared_ptr<Backend> Engine::find_backend(OpKind op, DType dtype) const {
  return backend_registry_.find_backend(op, dtype);
}

}  // namespace firstllm
