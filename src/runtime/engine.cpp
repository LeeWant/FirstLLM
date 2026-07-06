#include "firstllm/runtime/engine.h"

#include "firstllm/backends/cpu_backend.h"

#include <memory>
#include <utility>

namespace firstllm {

Engine::Engine(EngineConfig config)
    : config_(std::move(config)), backend_registry_() {}

Status Engine::initialize() {
  if (config_.enable_cpu_backend) {
    auto cpu_backend = std::make_shared<CpuBackend>();

    Status status = cpu_backend->initialize();
    if (!status.ok()) {
      return status;
    }

    backend_registry_.add_backend(std::move(cpu_backend));
  }

  return Status::Ok();
}

std::shared_ptr<Backend> Engine::find_backend(OpKind op, DType dtype) const {
  return backend_registry_.find_backend(op, dtype);
}

}  // namespace firstllm