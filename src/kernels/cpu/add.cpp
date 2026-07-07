#include "firstllm/kernels/cpu/add.h"

#include <cstring>

namespace firstllm {

// 最小 CPU add kernel：检查参数后按元素读取 float32、相加、写回 output。
Status CpuAdd(const Tensor& lhs, const Tensor& rhs, Tensor* output) {
  if (output == nullptr) {
    return Status::InvalidArgument("CpuAdd output tensor is null");
  }

  // 当前版本只支持 float32，后续如需 int/half 需要扩展这里和测试。
  if (lhs.dtype() != DType::kFloat32 ||
      rhs.dtype() != DType::kFloat32 ||
      output->dtype() != DType::kFloat32) {
    return Status::InvalidArgument("CpuAdd only supports float32 tensors");
  }

  if (lhs.shape().dims() != rhs.shape().dims() ||
      lhs.shape().dims() != output->shape().dims()) {
    return Status::InvalidArgument("CpuAdd requires matching shapes");
  }

  // kFloat32Size 是每个元素的字节跨度。
  constexpr std::size_t kFloat32Size = sizeof(float);
  // count 是需要处理的元素总数。
  const std::size_t count = lhs.num_elements();

  // i 是当前元素的一维线性索引，默认 tensor 是连续 row-major 内存。
  for (std::size_t i = 0; i < count; ++i) {
    // lhs_value/rhs_value 是从 raw bytes 中读出的两个输入值。
    float lhs_value = 0.0f;
    float rhs_value = 0.0f;

    // Tensor 当前只暴露 uint8_t*，因此用 memcpy 避免未对齐或别名问题。
    std::memcpy(&lhs_value, lhs.data() + i * kFloat32Size, kFloat32Size);
    std::memcpy(&rhs_value, rhs.data() + i * kFloat32Size, kFloat32Size);

    // output_value 是当前元素的加法结果。
    const float output_value = lhs_value + rhs_value;

    std::memcpy(output->data() + i * kFloat32Size,
                &output_value,
                kFloat32Size);
  }

  return Status::Ok();
}

}  // namespace firstllm
