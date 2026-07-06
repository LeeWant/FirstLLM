#include "firstllm/core/backend.h"

#include <utility>

namespace firstllm {

void BackendRegistry::add_backend(std::shared_ptr<Backend> backend) {
  backends_.push_back(std::move(backend));
}

const std::vector<std::shared_ptr<Backend>>& BackendRegistry::backends() const {
  return backends_;
}

std::shared_ptr<Backend> BackendRegistry::find_backend(OpKind op,
                                                       DType dtype) const {
  std::shared_ptr<Backend> best_backend;

  for (const auto& backend : backends_) {
    if (!backend) {
      continue;
    }

    if (!backend->is_available()) {
      continue;
    }

    if (!backend->supports(op, dtype)) {
      continue;
    }

    if (!best_backend ||
        backend->info().priority > best_backend->info().priority) {
      best_backend = backend;
    }
  }

  return best_backend;
}

}  // namespace firstllm