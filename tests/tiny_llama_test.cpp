#include "firstllm/model/tiny_llama.h"

#include <array>
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

// 浮点近似比较，forward 内部包含 sqrt、exp 和 softmax。
bool Near(float lhs, float rhs, float epsilon = 1e-5f) {
  return std::fabs(lhs - rhs) <= epsilon;
}

// 计算 3 维 logits 的 softmax 期望值，测试里用它对齐模型输出。
float SoftmaxValue(const std::array<float, 3>& logits, std::size_t index) {
  float max_value = logits[0];
  for (float value : logits) {
    if (value > max_value) {
      max_value = value;
    }
  }

  float sum = 0.0f;
  for (float value : logits) {
    sum += std::exp(value - max_value);
  }

  return std::exp(logits[index] - max_value) / sum;
}

}  // namespace

int main() {
  // hidden_states 是 2 个 token、每个 token 2 维 hidden。
  firstllm::Tensor hidden_states(firstllm::DType::kFloat32,
                                 firstllm::TensorShape({2, 2}));
  firstllm::Tensor norm_weight(firstllm::DType::kFloat32,
                               firstllm::TensorShape({2}));
  firstllm::Tensor output_projection(firstllm::DType::kFloat32,
                                     firstllm::TensorShape({2, 3}));
  firstllm::Tensor output_bias(firstllm::DType::kFloat32,
                               firstllm::TensorShape({3}));
  firstllm::Tensor probabilities(firstllm::DType::kFloat32,
                                 firstllm::TensorShape({2, 3}));

  WriteFloat(&hidden_states, 0, 3.0f);
  WriteFloat(&hidden_states, 1, 4.0f);
  WriteFloat(&hidden_states, 2, 0.0f);
  WriteFloat(&hidden_states, 3, 2.0f);

  // norm_weight 使用 1，让测试更直接观察 RMSNorm 自身的缩放结果。
  WriteFloat(&norm_weight, 0, 1.0f);
  WriteFloat(&norm_weight, 1, 1.0f);

  // output_projection 是 2x3：第 0 列取 hidden0，第 1/2 列取 hidden1。
  WriteFloat(&output_projection, 0, 1.0f);
  WriteFloat(&output_projection, 1, 0.0f);
  WriteFloat(&output_projection, 2, 0.0f);
  WriteFloat(&output_projection, 3, 0.0f);
  WriteFloat(&output_projection, 4, 1.0f);
  WriteFloat(&output_projection, 5, 1.0f);

  // output_bias 验证模型层会通过 CpuAdd 接上 bias。
  WriteFloat(&output_bias, 0, 0.0f);
  WriteFloat(&output_bias, 1, 0.5f);
  WriteFloat(&output_bias, 2, -0.5f);

  const firstllm::TinyLlamaConfig config{
      2,     // hidden_size
      3,     // vocab_size
      0.0f,  // epsilon 设为 0，方便手算期望值。
  };

  const firstllm::TinyLlamaWeights weights{
      &norm_weight,
      &output_projection,
      &output_bias,
  };

  const firstllm::TinyLlamaModel model(config, weights);

  const firstllm::Status ok = model.forward(hidden_states, &probabilities);

  assert(ok.ok());
  assert(model.config().hidden_size == 2);
  assert(model.weights().output_bias == &output_bias);

  // row0/row1 是手工推导出的 biased logits，再用 SoftmaxValue 计算期望概率。
  const float row0_scale = 1.0f / std::sqrt((9.0f + 16.0f) / 2.0f);
  const float row1_scale = 1.0f / std::sqrt((0.0f + 4.0f) / 2.0f);

  const std::array<float, 3> row0_logits{
      3.0f * row0_scale + 0.0f,
      4.0f * row0_scale + 0.5f,
      4.0f * row0_scale - 0.5f,
  };
  const std::array<float, 3> row1_logits{
      0.0f * row1_scale + 0.0f,
      2.0f * row1_scale + 0.5f,
      2.0f * row1_scale - 0.5f,
  };

  assert(Near(ReadFloat(probabilities, 0), SoftmaxValue(row0_logits, 0)));
  assert(Near(ReadFloat(probabilities, 1), SoftmaxValue(row0_logits, 1)));
  assert(Near(ReadFloat(probabilities, 2), SoftmaxValue(row0_logits, 2)));
  assert(Near(ReadFloat(probabilities, 3), SoftmaxValue(row1_logits, 0)));
  assert(Near(ReadFloat(probabilities, 4), SoftmaxValue(row1_logits, 1)));
  assert(Near(ReadFloat(probabilities, 5), SoftmaxValue(row1_logits, 2)));

  // 两行 softmax 的概率和都应接近 1。
  assert(Near(ReadFloat(probabilities, 0) +
              ReadFloat(probabilities, 1) +
              ReadFloat(probabilities, 2),
              1.0f));
  assert(Near(ReadFloat(probabilities, 3) +
              ReadFloat(probabilities, 4) +
              ReadFloat(probabilities, 5),
              1.0f));

  // 错误路径：output 指针不能为空。
  const firstllm::Status null_output =
      model.forward(hidden_states, nullptr);
  assert(!null_output.ok());
  assert(null_output.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：hidden_states 必须是 float32。
  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 2}));
  const firstllm::Status dtype_error =
      model.forward(wrong_dtype, &probabilities);
  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：hidden_states 必须是 2D。
  firstllm::Tensor wrong_rank(firstllm::DType::kFloat32,
                              firstllm::TensorShape({2, 1, 2}));
  const firstllm::Status rank_error =
      model.forward(wrong_rank, &probabilities);
  assert(!rank_error.ok());
  assert(rank_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：output shape 必须是 [tokens, vocab_size]。
  firstllm::Tensor wrong_output(firstllm::DType::kFloat32,
                                firstllm::TensorShape({2, 2}));
  const firstllm::Status output_error =
      model.forward(hidden_states, &wrong_output);
  assert(!output_error.ok());
  assert(output_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：权重引用不能为空。
  const firstllm::TinyLlamaWeights missing_weight{
      &norm_weight,
      &output_projection,
      nullptr,
  };
  const firstllm::TinyLlamaModel missing_weight_model(config,
                                                      missing_weight);
  const firstllm::Status missing_weight_error =
      missing_weight_model.forward(hidden_states, &probabilities);
  assert(!missing_weight_error.ok());
  assert(missing_weight_error.code() ==
         firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：output_bias 必须是一维 vocab_size。
  firstllm::Tensor wrong_bias(firstllm::DType::kFloat32,
                              firstllm::TensorShape({2, 3}));
  const firstllm::TinyLlamaWeights bad_bias_weights{
      &norm_weight,
      &output_projection,
      &wrong_bias,
  };
  const firstllm::TinyLlamaModel bad_bias_model(config, bad_bias_weights);
  const firstllm::Status bad_bias_error =
      bad_bias_model.forward(hidden_states, &probabilities);
  assert(!bad_bias_error.ok());
  assert(bad_bias_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：配置中的 hidden_size 不能为 0。
  const firstllm::TinyLlamaConfig bad_config{
      0,
      3,
      0.0f,
  };
  const firstllm::TinyLlamaModel bad_config_model(bad_config, weights);
  const firstllm::Status bad_config_error =
      bad_config_model.forward(hidden_states, &probabilities);
  assert(!bad_config_error.ok());
  assert(bad_config_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "tiny_llama_test passed\n";
  return 0;
}
