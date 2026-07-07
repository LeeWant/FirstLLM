#include "firstllm/kernels/cpu/softmax.h"

#include <cassert>
#include <cmath>
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

// 浮点近似比较，避免直接用 == 比较 softmax 结果。
bool Near(float lhs, float rhs, float epsilon = 1e-5f) {
  return std::fabs(lhs - rhs) <= epsilon;
}

}  // namespace

int main() {
  // input/output 都是 2x3，softmax 会沿最后一维分别处理两行。
  firstllm::Tensor input(firstllm::DType::kFloat32,
                         firstllm::TensorShape({2, 3}));
  firstllm::Tensor output(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 3}));

  WriteFloat(&input, 0, 1.0f);
  WriteFloat(&input, 1, 2.0f);
  WriteFloat(&input, 2, 3.0f);

  // 第二行使用 1000，验证 max-subtraction 能避免 exp 溢出。
  WriteFloat(&input, 3, 1000.0f);
  WriteFloat(&input, 4, 1000.0f);
  WriteFloat(&input, 5, 1000.0f);

  const firstllm::Status ok =
      firstllm::CpuSoftmaxLastDim(input, &output);

  assert(ok.ok());

  // 第一行是普通 softmax 结果。
  assert(Near(ReadFloat(output, 0), 0.09003057f));
  assert(Near(ReadFloat(output, 1), 0.24472848f));
  assert(Near(ReadFloat(output, 2), 0.66524094f));

  // 第二行三个输入相等，概率应为均匀分布。
  assert(Near(ReadFloat(output, 3), 1.0f / 3.0f));
  assert(Near(ReadFloat(output, 4), 1.0f / 3.0f));
  assert(Near(ReadFloat(output, 5), 1.0f / 3.0f));

  // row0_sum 验证第一行概率和为 1。
  const float row0_sum =
      ReadFloat(output, 0) + ReadFloat(output, 1) + ReadFloat(output, 2);
  // row1_sum 验证第二行概率和为 1。
  const float row1_sum =
      ReadFloat(output, 3) + ReadFloat(output, 4) + ReadFloat(output, 5);

  assert(Near(row0_sum, 1.0f));
  assert(Near(row1_sum, 1.0f));

  const firstllm::Status null_output =
      firstllm::CpuSoftmaxLastDim(input, nullptr);

  // 错误路径：output 指针不能为空。
  assert(!null_output.ok());
  assert(null_output.code() == firstllm::ErrorCode::kInvalidArgument);

  // wrong_dtype 故意使用 int32，验证 dtype 检查。
  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 3}));

  const firstllm::Status dtype_error =
      firstllm::CpuSoftmaxLastDim(wrong_dtype, &output);

  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // wrong_shape 故意使用不同 shape，验证输入输出 shape 必须一致。
  firstllm::Tensor wrong_shape(firstllm::DType::kFloat32,
                               firstllm::TensorShape({3, 2}));

  const firstllm::Status shape_error =
      firstllm::CpuSoftmaxLastDim(input, &wrong_shape);

  assert(!shape_error.ok());
  assert(shape_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "cpu_softmax_test passed\n";
  return 0;
}
