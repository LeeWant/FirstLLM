#include "firstllm/kernels/cpu/matmul.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace {

// 向 Tensor 的 raw bytes 写入一个 float32 测试值。
void WriteFloat(firstllm::Tensor* tensor, std::size_t index, float value) {
  std::memcpy(tensor->data() + index * sizeof(float),
              &value,
              sizeof(float));
}

// 从 Tensor 的 raw bytes 读取一个 float32 结果。
float ReadFloat(const firstllm::Tensor& tensor, std::size_t index) {
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

}  // namespace

int main() {
  // lhs 是 2x3，rhs 是 3x2，output 应是 2x2。
  firstllm::Tensor lhs(firstllm::DType::kFloat32,
                       firstllm::TensorShape({2, 3}));
  firstllm::Tensor rhs(firstllm::DType::kFloat32,
                       firstllm::TensorShape({3, 2}));
  firstllm::Tensor output(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 2}));

  WriteFloat(&lhs, 0, 1.0f);
  WriteFloat(&lhs, 1, 2.0f);
  WriteFloat(&lhs, 2, 3.0f);
  WriteFloat(&lhs, 3, 4.0f);
  WriteFloat(&lhs, 4, 5.0f);
  WriteFloat(&lhs, 5, 6.0f);

  WriteFloat(&rhs, 0, 7.0f);
  WriteFloat(&rhs, 1, 8.0f);
  WriteFloat(&rhs, 2, 9.0f);
  WriteFloat(&rhs, 3, 10.0f);
  WriteFloat(&rhs, 4, 11.0f);
  WriteFloat(&rhs, 5, 12.0f);

  const firstllm::Status ok = firstllm::CpuMatMul(lhs, rhs, &output);

  // 正常路径：验证四个矩阵乘法结果。
  assert(ok.ok());
  assert(ReadFloat(output, 0) == 58.0f);
  assert(ReadFloat(output, 1) == 64.0f);
  assert(ReadFloat(output, 2) == 139.0f);
  assert(ReadFloat(output, 3) == 154.0f);

  const firstllm::Status null_output =
      firstllm::CpuMatMul(lhs, rhs, nullptr);

  // 错误路径：output 指针不能为空。
  assert(!null_output.ok());
  assert(null_output.code() == firstllm::ErrorCode::kInvalidArgument);

  // wrong_dtype 故意使用 int32，验证 dtype 检查。
  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 3}));

  const firstllm::Status dtype_error =
      firstllm::CpuMatMul(wrong_dtype, rhs, &output);

  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // wrong_rank 故意使用 3D tensor，验证 rank 检查。
  firstllm::Tensor wrong_rank(firstllm::DType::kFloat32,
                              firstllm::TensorShape({2, 3, 1}));

  const firstllm::Status rank_error =
      firstllm::CpuMatMul(wrong_rank, rhs, &output);

  assert(!rank_error.ok());
  assert(rank_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // wrong_inner 故意让 lhs 的 k 和 rhs 的 k 不匹配。
  firstllm::Tensor wrong_inner(firstllm::DType::kFloat32,
                               firstllm::TensorShape({4, 2}));

  const firstllm::Status inner_error =
      firstllm::CpuMatMul(lhs, wrong_inner, &output);

  assert(!inner_error.ok());
  assert(inner_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // wrong_output 故意使用错误输出 shape。
  firstllm::Tensor wrong_output(firstllm::DType::kFloat32,
                                firstllm::TensorShape({2, 3}));

  const firstllm::Status output_error =
      firstllm::CpuMatMul(lhs, rhs, &wrong_output);

  assert(!output_error.ok());
  assert(output_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "cpu_matmul_test passed\n";
  return 0;
}
