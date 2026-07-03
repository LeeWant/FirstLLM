#include "firstllm/core/status.h"

#include <utility>

namespace firstllm {

Status::Status() : code_(ErrorCode::kOk), message_() {}

Status::Status(ErrorCode code, std::string message)
    : code_(code), message_(std::move(message)) {}

Status Status::Ok() {
  return Status();
}

Status Status::InvalidArgument(std::string message) {
  return Status(ErrorCode::kInvalidArgument, std::move(message));
}

Status Status::NotFound(std::string message) {
  return Status(ErrorCode::kNotFound, std::move(message));
}

Status Status::BackendUnavailable(std::string message) {
  return Status(ErrorCode::kBackendUnavailable, std::move(message));
}

Status Status::Internal(std::string message) {
  return Status(ErrorCode::kInternal, std::move(message));
}

bool Status::ok() const {
  return code_ == ErrorCode::kOk;
}

ErrorCode Status::code() const {
  return code_;
}

const std::string& Status::message() const {
  return message_;
}

}  // namespace firstllm