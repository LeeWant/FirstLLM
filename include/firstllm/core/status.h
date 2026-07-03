#pragma once

#include <string>

namespace firstllm {

enum class ErrorCode {
  kOk = 0,
  kInvalidArgument,
  kNotFound,
  kBackendUnavailable,
  kInternal,
};

class Status {
 public:
  Status();

  Status(ErrorCode code, std::string message);

  static Status Ok();
  static Status InvalidArgument(std::string message);
  static Status NotFound(std::string message);
  static Status BackendUnavailable(std::string message);
  static Status Internal(std::string message);

  bool ok() const;
  ErrorCode code() const;
  const std::string& message() const;

 private:
  ErrorCode code_;
  std::string message_;
};

}  // namespace firstllm