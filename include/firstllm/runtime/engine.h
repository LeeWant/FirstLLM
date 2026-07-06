#pragma once

#include "firstllm/core/backend.h"
#include "firstllm/core/status.h"

#include <memory>

namespace firstllm {

struct EngineConfig {
  bool enable_cpu_backend = true;
};

class Engine {
 public:
  explicit Engine(EngineConfig config);

  Status initialize();

  std::shared_ptr<Backend> find_backend(OpKind op, DType dtype) const;

 private:
  EngineConfig config_;
  BackendRegistry backend_registry_;
};

}  // namespace firstllm