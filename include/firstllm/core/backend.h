#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

#include <memory>
#include <string>
#include <vector>

namespace firstllm {

// 计算设备类型。当前 CPU 可用，CUDA 先以可选 backend 骨架存在。
enum class DeviceType {
  kCpu,   // 普通主机 CPU。
  kCuda,  // NVIDIA CUDA 设备；当前还没有真实 CUDA kernel。
};

// 设备描述：设备类型 + 同类设备下的序号。
struct Device {
  DeviceType type;  // 设备类型，例如 CPU 或 CUDA。
  int index;        // 设备编号；单 CPU 当前使用 0。
};

// 后端可以声明支持的算子种类。
enum class OpKind {
  kAdd,      // 逐元素加法。
  kMatMul,   // 二维矩阵乘法。
  kSoftmax,  // 最后一维 softmax。
  kRmsNorm,  // RMSNorm 归一化。
};

// backend 的基本信息，用于展示和优先级选择。
struct BackendInfo {
  std::string name;  // backend 名称，例如 "cpu" 或 "cuda"。
  Device device;     // backend 对应的设备。
  int priority;      // 优先级；数值越大，Registry 越优先选择。
};

// Backend 是能力提供者接口：回答“我是谁、是否可用、支持什么”。
class Backend {
 public:
  virtual ~Backend() = default;

  // 返回 backend 的静态信息。
  virtual BackendInfo info() const = 0;

  // 初始化 backend；失败时返回 Status 描述原因。
  virtual Status initialize() = 0;

  // 返回 backend 当前是否可被调度。
  virtual bool is_available() const = 0;

  // 查询 backend 是否支持指定 op + dtype 组合。
  virtual bool supports(OpKind op, DType dtype) const = 0;
};

// BackendRegistry 保存多个 backend，并按能力和优先级选择可用 backend。
class BackendRegistry {
 public:
  // 注册一个 backend；允许空指针传入，但查找时会跳过空指针。
  void add_backend(std::shared_ptr<Backend> backend);

  // 返回已注册 backend 列表。
  const std::vector<std::shared_ptr<Backend>>& backends() const;

  // 在可用且支持目标能力的 backend 中，返回优先级最高者。
  std::shared_ptr<Backend> find_backend(OpKind op, DType dtype) const;

 private:
  // 已注册 backend 列表，顺序保留注册顺序；选择时再比较 priority。
  std::vector<std::shared_ptr<Backend>> backends_;
};

}  // namespace firstllm
