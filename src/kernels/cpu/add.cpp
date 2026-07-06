#include "firstllm/kernels/cpu/add.h"

#include <cstring>

namespace firstllm {

Status CpuAdd(const Tensor& lhs, const Tensor& rhs, Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuAdd output tensor is null");
  }

  if (lhs.dtype() != DType::kFloat32 ||
      rhs.dtype() != DType::kFloat32 ||
      output->dtype() != DType::kFloat32) {
    return Status::InvalidArgument("CpuAdd only supports float32 tensors");
  }

  if (lhs.shape().dims() != rhs.shape().dims() ||
      lhs.shape().dims() != output->shape().dims()) {
    return Status::InvalidArgument("CpuAdd requires matching shapes");
  }

  constexpr std::size_t kFloat32Size = sizeof(float);
  const std::size_t count = lhs.num_elements();

  for (std::size_t i = 0; i < count; ++i) {
    float lhs_value = 0.0f;
    float rhs_value = 0.0f;

    std::memcpy(&lhs_value, lhs.data() + i * kFloat32Size, kFloat32Size);
    std::memcpy(&rhs_value, rhs.data() + i * kFloat32Size, kFloat32Size);

    const float output_value = lhs_value + rhs_value;

    std::memcpy(output->data() + i * kFloat32Size,
                &output_value,
                kFloat32Size);
  }

  return Status::Ok();
}

}  // namespace firstllm