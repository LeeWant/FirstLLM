#include "firstllm/core/status.h"

#include <utility>

namespace firstllm {

// 默认成功状态：错误码为 kOk，错误信息为空。
Status::Status() : code_(ErrorCode::kOk), message_() {}

// 通用构造函数：message 使用 move，避免不必要的字符串拷贝。
Status::Status(ErrorCode code, std::string message)
    : code_(code), message_(std::move(message)) {}

// 成功状态工厂。
Status Status::Ok() {
  return Status();
}

// 参数错误工厂。
Status Status::InvalidArgument(std::string message) {
  return Status(ErrorCode::kInvalidArgument, std::move(message));
}

// 资源未找到工厂。
Status Status::NotFound(std::string message) {
  return Status(ErrorCode::kNotFound, std::move(message));
}

// backend 不可用工厂。
Status Status::BackendUnavailable(std::string message) {
  return Status(ErrorCode::kBackendUnavailable, std::move(message));
}

// 内部错误工厂。
Status Status::Internal(std::string message) {
  return Status(ErrorCode::kInternal, std::move(message));
}

// 只有 kOk 表示成功，其它错误码都表示失败。
bool Status::ok() const {
  return code_ == ErrorCode::kOk;
}

// 暴露错误码，测试和调用方可据此分支处理。
ErrorCode Status::code() const {
  return code_;
}

// 暴露错误信息，主要用于日志、调试和测试失败时定位问题。
const std::string& Status::message() const {
  return message_;
}

}  // namespace firstllm
