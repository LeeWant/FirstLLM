#pragma once

#include "firstllm/core/backend.h"

namespace firstllm {

class CudaBackend : public Backend {
 public:
  CudaBackend();

  BackendInfo info() const override;
  Status initialize() override;
  bool is_available() const override;
  bool supports(OpKind op, DType dtype) const override;

 private:
  BackendInfo info_;
  bool initialized_;
};

}  // namespace firstllm