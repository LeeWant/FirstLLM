#include "firstllm/core/backend.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

class FakeBackend : public firstllm::Backend {
 public:
  FakeBackend(std::string name,
              firstllm::DeviceType device_type,
              int priority,
              bool available,
              firstllm::OpKind supported_op,
              firstllm::DType supported_dtype)
      : info_{std::move(name), firstllm::Device{device_type, 0}, priority},
        available_(available),
        supported_op_(supported_op),
        supported_dtype_(supported_dtype) {}

  firstllm::BackendInfo info() const override {
    return info_;
  }

  firstllm::Status initialize() override {
    return firstllm::Status::Ok();
  }

  bool is_available() const override {
    return available_;
  }

  bool supports(firstllm::OpKind op, firstllm::DType dtype) const override {
    return op == supported_op_ && dtype == supported_dtype_;
  }

 private:
  firstllm::BackendInfo info_;
  bool available_;
  firstllm::OpKind supported_op_;
  firstllm::DType supported_dtype_;
};

int main() {
  firstllm::BackendRegistry registry;

  assert(registry.backends().empty());

  registry.add_backend(std::make_shared<FakeBackend>(
      "cpu-low",
      firstllm::DeviceType::kCpu,
      10,
      true,
      firstllm::OpKind::kAdd,
      firstllm::DType::kFloat32));

  registry.add_backend(std::make_shared<FakeBackend>(
      "cuda-high",
      firstllm::DeviceType::kCuda,
      100,
      true,
      firstllm::OpKind::kAdd,
      firstllm::DType::kFloat32));

  registry.add_backend(std::make_shared<FakeBackend>(
      "unavailable",
      firstllm::DeviceType::kCpu,
      1000,
      false,
      firstllm::OpKind::kAdd,
      firstllm::DType::kFloat32));

  assert(registry.backends().size() == 3);

  const auto backend =
      registry.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

  assert(backend != nullptr);
  assert(backend->info().name == "cuda-high");
  assert(backend->info().priority == 100);

  const auto missing = registry.find_backend(firstllm::OpKind::kMatMul,
                                             firstllm::DType::kFloat32);

  assert(missing == nullptr);

  std::cout << "backend_test passed\n";
  return 0;
}