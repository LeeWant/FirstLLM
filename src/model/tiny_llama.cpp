#include "firstllm/model/tiny_llama.h"

#include "firstllm/kernels/cpu/add.h"
#include "firstllm/kernels/cpu/matmul.h"
#include "firstllm/kernels/cpu/rms_norm.h"
#include "firstllm/kernels/cpu/softmax.h"

#include <cstring>
#include <utility>

namespace firstllm {
namespace {

// 从 Tensor raw bytes 中读取 float32。当前模型层只支持 float32 权重和激活。
float ReadFloat(const Tensor& tensor, std::size_t index) {
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

// 向 Tensor raw bytes 写入 float32，用于构造 broadcast 后的 bias 临时张量。
void WriteFloat(Tensor* tensor, std::size_t index, float value) {
  std::memcpy(tensor->data() + index * sizeof(float),
              &value,
              sizeof(float));
}

// 检查第 12 章极简模型配置。完整 Llama 配置后续会单独建结构。
Status ValidateConfig(const TinyLlamaConfig& config) {
  if (config.hidden_size == 0) {
    return Status::InvalidArgument("TinyLlama hidden_size must be non-zero");
  }

  if (config.vocab_size == 0) {
    return Status::InvalidArgument("TinyLlama vocab_size must be non-zero");
  }

  if (config.rms_norm_epsilon < 0.0f) {
    return Status::InvalidArgument(
        "TinyLlama rms_norm_epsilon must be non-negative");
  }

  return Status::Ok();
}

// 检查输入和输出 tensor 的 dtype、rank 和形状。
Status ValidateActivations(const TinyLlamaConfig& config,
                           const Tensor& hidden_states,
                           const Tensor* probabilities) {
  if (probabilities == nullptr) {
    return Status::InvalidArgument("TinyLlama output tensor is null");
  }

  if (hidden_states.dtype() != DType::kFloat32 ||
      probabilities->dtype() != DType::kFloat32) {
    return Status::InvalidArgument(
        "TinyLlama forward only supports float32 tensors");
  }

  if (hidden_states.shape().rank() != 2) {
    return Status::InvalidArgument(
        "TinyLlama hidden_states must be 2D");
  }

  if (probabilities->shape().rank() != 2) {
    return Status::InvalidArgument(
        "TinyLlama probabilities output must be 2D");
  }

  const std::size_t token_count = hidden_states.shape().dim(0);
  const std::size_t hidden_size = hidden_states.shape().dim(1);

  if (token_count == 0) {
    return Status::InvalidArgument(
        "TinyLlama token count must be non-zero");
  }

  if (hidden_size != config.hidden_size) {
    return Status::InvalidArgument(
        "TinyLlama hidden size does not match config");
  }

  if (probabilities->shape().dim(0) != token_count ||
      probabilities->shape().dim(1) != config.vocab_size) {
    return Status::InvalidArgument(
        "TinyLlama probabilities output shape is invalid");
  }

  return Status::Ok();
}

// 检查权重是否存在，并确认它们符合当前最小 forward 的形状要求。
Status ValidateWeights(const TinyLlamaConfig& config,
                       const TinyLlamaWeights& weights) {
  if (weights.final_norm_weight == nullptr ||
      weights.output_projection == nullptr ||
      weights.output_bias == nullptr) {
    return Status::InvalidArgument("TinyLlama weights must not be null");
  }

  const Tensor& norm_weight = *weights.final_norm_weight;
  const Tensor& projection = *weights.output_projection;
  const Tensor& bias = *weights.output_bias;

  if (norm_weight.dtype() != DType::kFloat32 ||
      projection.dtype() != DType::kFloat32 ||
      bias.dtype() != DType::kFloat32) {
    return Status::InvalidArgument(
        "TinyLlama weights only support float32 tensors");
  }

  if (norm_weight.shape().rank() != 1 ||
      norm_weight.shape().dim(0) != config.hidden_size) {
    return Status::InvalidArgument(
        "TinyLlama final_norm_weight shape is invalid");
  }

  if (projection.shape().rank() != 2 ||
      projection.shape().dim(0) != config.hidden_size ||
      projection.shape().dim(1) != config.vocab_size) {
    return Status::InvalidArgument(
        "TinyLlama output_projection shape is invalid");
  }

  if (bias.shape().rank() != 1 ||
      bias.shape().dim(0) != config.vocab_size) {
    return Status::InvalidArgument(
        "TinyLlama output_bias shape is invalid");
  }

  return Status::Ok();
}

// 当前 CpuAdd 不支持 broadcast，所以模型层先把 [vocab] bias 展开成 [tokens, vocab]。
void ExpandBias(const Tensor& bias,
                std::size_t token_count,
                std::size_t vocab_size,
                Tensor* expanded_bias) {
  for (std::size_t token = 0; token < token_count; ++token) {
    for (std::size_t vocab = 0; vocab < vocab_size; ++vocab) {
      const float value = ReadFloat(bias, vocab);
      WriteFloat(expanded_bias, token * vocab_size + vocab, value);
    }
  }
}

}  // namespace

// 保存配置和权重引用；权重 Tensor 的生命周期由调用方负责。
TinyLlamaModel::TinyLlamaModel(TinyLlamaConfig config,
                               TinyLlamaWeights weights)
    : config_(std::move(config)), weights_(weights) {}

// 返回模型配置。
const TinyLlamaConfig& TinyLlamaModel::config() const {
  return config_;
}

// 返回权重引用集合。
const TinyLlamaWeights& TinyLlamaModel::weights() const {
  return weights_;
}

// 极简 forward：RMSNorm -> MatMul -> Add bias -> Softmax。
Status TinyLlamaModel::forward(const Tensor& hidden_states,
                               Tensor* probabilities) const {
  Status status = ValidateConfig(config_);
  if (!status.ok()) {
    return status;
  }

  status = ValidateActivations(config_, hidden_states, probabilities);
  if (!status.ok()) {
    return status;
  }

  status = ValidateWeights(config_, weights_);
  if (!status.ok()) {
    return status;
  }

  const std::size_t token_count = hidden_states.shape().dim(0);

  // normalized 保存 RMSNorm 后的 hidden states。
  Tensor normalized(DType::kFloat32,
                    TensorShape({token_count, config_.hidden_size}));
  status = CpuRmsNorm(hidden_states,
                      *weights_.final_norm_weight,
                      config_.rms_norm_epsilon,
                      &normalized);
  if (!status.ok()) {
    return status;
  }

  // logits 保存投影到 vocab 维度后的原始分数。
  Tensor logits(DType::kFloat32,
                TensorShape({token_count, config_.vocab_size}));
  status = CpuMatMul(normalized, *weights_.output_projection, &logits);
  if (!status.ok()) {
    return status;
  }

  // expanded_bias 是 [vocab] bias 的逐 token 展开版本，供 CpuAdd 复用。
  Tensor expanded_bias(DType::kFloat32,
                       TensorShape({token_count, config_.vocab_size}));
  ExpandBias(*weights_.output_bias,
             token_count,
             config_.vocab_size,
             &expanded_bias);

  // biased_logits 是加上输出 bias 后的 logits。
  Tensor biased_logits(DType::kFloat32,
                       TensorShape({token_count, config_.vocab_size}));
  status = CpuAdd(logits, expanded_bias, &biased_logits);
  if (!status.ok()) {
    return status;
  }

  // 第一版直接输出概率分布，后续 sampler 可以接在这里之后。
  return CpuSoftmaxLastDim(biased_logits, probabilities);
}

}  // namespace firstllm
