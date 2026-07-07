#include "firstllm/core/backend.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

// FakeBackend 是测试专用 backend，用来验证 BackendRegistry 的选择逻辑。
class FakeBackend : public firstllm::Backend {
 public:
  // 构造一个可配置的 backend：名字、设备、优先级、可用性和支持能力都可指定。
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

  // 返回构造时保存的 backend 信息。
  firstllm::BackendInfo info() const override {
    return info_;
  }

  // 测试 backend 不需要真实初始化，直接成功。
  firstllm::Status initialize() override {
    return firstllm::Status::Ok();
  }

  // 返回构造时指定的可用性。
  bool is_available() const override {
    return available_;
  }

  // 只有构造时指定的 op + dtype 会被声明为支持。
  bool supports(firstllm::OpKind op, firstllm::DType dtype) const override {
    return op == supported_op_ && dtype == supported_dtype_;
  }

 private:
  firstllm::BackendInfo info_;          // backend 名称、设备和优先级。
  bool available_;                      // 是否可被 registry 选择。
  firstllm::OpKind supported_op_;       // 唯一支持的算子。
  firstllm::DType supported_dtype_;     // 唯一支持的数据类型。
};

int main() {
  // registry 初始应为空。
  firstllm::BackendRegistry registry;

  assert(registry.backends().empty());

  // cpu-low 可用但优先级较低。
  registry.add_backend(std::make_shared<FakeBackend>(
      "cpu-low",
      firstllm::DeviceType::kCpu,
      10,
      true,
      firstllm::OpKind::kAdd,
      firstllm::DType::kFloat32));

  // cuda-high 可用且优先级较高，应当被选中。
  registry.add_backend(std::make_shared<FakeBackend>(
      "cuda-high",
      firstllm::DeviceType::kCuda,
      100,
      true,
      firstllm::OpKind::kAdd,
      firstllm::DType::kFloat32));

  // unavailable 优先级最高，但不可用，必须被跳过。
  registry.add_backend(std::make_shared<FakeBackend>(
      "unavailable",
      firstllm::DeviceType::kCpu,
      1000,
      false,
      firstllm::OpKind::kAdd,
      firstllm::DType::kFloat32));

  assert(registry.backends().size() == 3);

  // backend 是 registry 针对 float32 add 找到的最佳候选。
  const auto backend =
      registry.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

  assert(backend != nullptr);
  assert(backend->info().name == "cuda-high");
  assert(backend->info().priority == 100);

  // missing 验证不支持的 op 不会返回 backend。
  const auto missing = registry.find_backend(firstllm::OpKind::kMatMul,
                                             firstllm::DType::kFloat32);

  assert(missing == nullptr);

  std::cout << "backend_test passed\n";
  return 0;
}
