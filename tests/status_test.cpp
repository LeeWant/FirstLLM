#include "firstllm/core/status.h"

#include <cassert>
#include <iostream>

int main() {
    const firstllm::Status ok = firstllm::Status::Ok();

    assert(ok.ok());
    assert(ok.code() == firstllm::ErrorCode::kOk);
    assert(ok.message().empty());

    const firstllm::Status invalid = 
        firstllm::Status::InvalidArgument("形状不匹配：Shape mismatch");
    
    assert(!invalid.ok());
    assert(invalid.code() == firstllm::ErrorCode::kInvalidArgument);
    assert(invalid.message() == "形状不匹配：Shape mismatch");

    std::cout << "status_test passed\n";
    return 0;
}