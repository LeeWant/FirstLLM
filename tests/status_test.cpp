#include "firstllm/core/status.h"

#include <cassert>
#include <iostream>

int main() {
  // ok 表示成功状态，应该没有错误码和错误信息。
  const firstllm::Status ok = firstllm::Status::Ok();

  assert(ok.ok());
  assert(ok.code() == firstllm::ErrorCode::kOk);
  assert(ok.message().empty());

  // invalid 表示参数错误，用来验证错误码和中文错误信息能被保留下来。
  const firstllm::Status invalid =
      firstllm::Status::InvalidArgument("形状不匹配：Shape mismatch");

  assert(!invalid.ok());
  assert(invalid.code() == firstllm::ErrorCode::kInvalidArgument);
  assert(invalid.message() == "形状不匹配：Shape mismatch");

  std::cout << "status_test passed\n";
  return 0;
}
