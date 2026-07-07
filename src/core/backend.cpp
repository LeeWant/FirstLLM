#include "firstllm/core/backend.h"

#include <utility>

namespace firstllm {

// 注册 backend。这里不主动 initialize，初始化责任留给调用方。
void BackendRegistry::add_backend(std::shared_ptr<Backend> backend) {
  backends_.push_back(std::move(backend));
}

// 返回已注册 backend 列表，主要用于测试和调试。
const std::vector<std::shared_ptr<Backend>>& BackendRegistry::backends() const {
  return backends_;
}

// 按可用性、能力和优先级查找最合适的 backend。
std::shared_ptr<Backend> BackendRegistry::find_backend(OpKind op,
                                                       DType dtype) const {
  // best_backend 保存当前找到的最高优先级候选。
  std::shared_ptr<Backend> best_backend;

  // backend 是当前正在检查的候选 backend。
  for (const auto& backend : backends_) {
    // 允许注册表里存在空指针，查找时跳过即可。
    if (!backend) {
      continue;
    }

    // 不可用 backend 不能参与调度，例如当前 CUDA 骨架。
    if (!backend->is_available()) {
      continue;
    }

    // 只选择声明支持目标 op + dtype 的 backend。
    if (!backend->supports(op, dtype)) {
      continue;
    }

    // priority 越高越优先；相同优先级时保留先遇到的 backend。
    if (!best_backend ||
        backend->info().priority > best_backend->info().priority) {
      best_backend = backend;
    }
  }

  return best_backend;
}

}  // namespace firstllm
