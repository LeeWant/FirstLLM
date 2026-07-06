#include "firstllm/firstllm.h"

#include <iostream>

int main() {
    firstllm::Engine engine(firstllm::EngineConfig{});

    const firstllm::Status status = engine.initialize();

    if (!status.ok()) {
        std::cout << "FirstLLM initialize failed: "
                  << status.message() << "\n";
        return 1;
    }

    const auto backend = engine.find_backend(firstllm::OpKind::kAdd, firstllm::DType::kFloat32);

    if (!backend) {
        std::cout << "FirstLLM initialized, but no backend found\n";
        return 1;
    }

    const firstllm::BackendInfo info = backend->info();

    std::cout << "FirstLLM initialized\n";
    std::cout << "Selected backend: " << info.name << "\n";

    return 0;
}