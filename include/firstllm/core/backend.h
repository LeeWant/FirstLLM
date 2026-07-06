#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

#include <memory>
#include <string>
#include <vector>

namespace firstllm {

enum class DeviceType {
  kCpu,
  kCuda,
};

struct Device {
  DeviceType type;
  int index;
};

enum class OpKind {
  kAdd,
  kMatMul,
  kSoftmax,
  kRmsNorm,
};

struct BackendInfo {
  std::string name;
  Device device;
  int priority;
};

class Backend {
 public:
  virtual ~Backend() = default;

  virtual BackendInfo info() const = 0;
  virtual Status initialize() = 0;
  virtual bool is_available() const = 0;
  virtual bool supports(OpKind op, DType dtype) const = 0;
};

class BackendRegistry {
 public:
  void add_backend(std::shared_ptr<Backend> backend);

  const std::vector<std::shared_ptr<Backend>>& backends() const;

  std::shared_ptr<Backend> find_backend(OpKind op, DType dtype) const;

 private:
  std::vector<std::shared_ptr<Backend>> backends_;
};

}  // namespace firstllm