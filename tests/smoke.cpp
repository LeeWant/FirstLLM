#include "firstllm/firstllm.h"

#include <cassert>
#include <iostream>

int main() {
    firstllm::Engine engine(firstllm::EngineConfig{});

    const firstllm::Status status = engine.initialize();

    assert(status.ok());

    const auto backend = engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

    assert(backend != nullptr);
    assert(backend->info().name == "cpu");

    firstllm::Tensor tensor(firstllm::DType::kFloat32,
                            firstllm::TensorShape({2,2}));
    
    assert(tensor.num_elements() == 4);
    assert(tensor.byte_size() == 16);

    std::cout << "smoke_test passed\n";
    return 0;
}