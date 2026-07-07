#pragma once

#include <string>

namespace firstllm {

// 统一错误码：项目内所有模块都用它表达失败原因，而不是只返回 bool。
enum class ErrorCode {
  kOk = 0,               // 操作成功。
  kInvalidArgument,      // 调用方传入了非法参数，例如 shape 不匹配。
  kNotFound,             // 资源不存在，例如 GGUF 文件路径错误。
  kBackendUnavailable,   // backend 当前不可用，例如 CUDA 骨架尚未启用。
  kInternal,             // 内部错误：暂时无法归类到更具体错误时使用。
};

// Status 是 FirstLLM 的最小错误处理对象：错误码 + 可读错误信息。
class Status {
 public:
  // 默认构造成功状态，等价于 Status::Ok()。
  Status();

  // 用指定错误码和错误信息构造状态；通常优先使用下面的静态工厂函数。
  Status(ErrorCode code, std::string message);

  // 构造成功状态。
  static Status Ok();

  // 构造参数错误状态。
  static Status InvalidArgument(std::string message);

  // 构造资源未找到状态。
  static Status NotFound(std::string message);

  // 构造 backend 不可用状态。
  static Status BackendUnavailable(std::string message);

  // 构造内部错误状态。
  static Status Internal(std::string message);

  // 判断当前状态是否成功。
  bool ok() const;

  // 返回错误码；成功时为 ErrorCode::kOk。
  ErrorCode code() const;

  // 返回错误信息；成功状态下通常为空字符串。
  const std::string& message() const;

 private:
  ErrorCode code_;       // 当前状态码，决定 ok() 的结果。
  std::string message_;  // 给人阅读的错误说明，方便定位问题。
};

}  // namespace firstllm
