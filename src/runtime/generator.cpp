#include "firstllm/runtime/generator.h"

#include <cstring>
#include <limits>

namespace firstllm {
namespace {

// 从 Tensor raw bytes 中读取 float32 概率值。
float ReadFloat(const Tensor& tensor, std::size_t index) {
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

// 检查生成配置是否可用于保存至少一个 token。
Status ValidateGeneratorConfig(const GeneratorConfig& config) {
  if (config.max_token_count == 0) {
    return Status::InvalidArgument(
        "Generator max_token_count must be non-zero");
  }

  if (config.stop_on_eos && config.eos_token_id < 0) {
    return Status::InvalidArgument(
        "Generator eos_token_id must be non-negative when stop_on_eos is true");
  }

  return Status::Ok();
}

// 检查 hidden_state 是否表示单个 token 的 hidden 向量。
Status ValidateOneStepHiddenState(const TinyLlamaModel& model,
                                  const Tensor& hidden_state) {
  if (hidden_state.dtype() != DType::kFloat32) {
    return Status::InvalidArgument(
        "Generator hidden_state must be float32");
  }

  if (hidden_state.shape().rank() != 2) {
    return Status::InvalidArgument(
        "Generator hidden_state must be 2D");
  }

  if (hidden_state.shape().dim(0) != 1) {
    return Status::InvalidArgument(
        "Generator hidden_state must contain exactly one token");
  }

  if (hidden_state.shape().dim(1) != model.config().hidden_size) {
    return Status::InvalidArgument(
        "Generator hidden_state hidden size is invalid");
  }

  return Status::Ok();
}

}  // namespace

// Greedy 选择最后一行概率最大的 token id；相同概率时保留更小的 token id。
Status GreedyNextToken(const Tensor& probabilities,
                       std::int32_t* token_id) {
  if (token_id == nullptr) {
    return Status::InvalidArgument("GreedyNextToken token_id pointer is null");
  }

  if (probabilities.dtype() != DType::kFloat32) {
    return Status::InvalidArgument(
        "GreedyNextToken only supports float32 probabilities");
  }

  if (probabilities.shape().rank() != 2) {
    return Status::InvalidArgument(
        "GreedyNextToken probabilities must be 2D");
  }

  const std::size_t token_count = probabilities.shape().dim(0);
  const std::size_t vocab_size = probabilities.shape().dim(1);

  if (token_count == 0) {
    return Status::InvalidArgument(
        "GreedyNextToken token count must be non-zero");
  }

  if (vocab_size == 0) {
    return Status::InvalidArgument(
        "GreedyNextToken vocab size must be non-zero");
  }

  if (vocab_size >
      static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
    return Status::InvalidArgument(
        "GreedyNextToken vocab size exceeds int32 token id range");
  }

  // 只从最后一个 token 的概率分布中选 next token。
  const std::size_t row_offset = (token_count - 1) * vocab_size;
  float best_value = ReadFloat(probabilities, row_offset);
  std::size_t best_index = 0;

  for (std::size_t vocab_index = 1; vocab_index < vocab_size;
       ++vocab_index) {
    const float value =
        ReadFloat(probabilities, row_offset + vocab_index);
    if (value > best_value) {
      best_value = value;
      best_index = vocab_index;
    }
  }

  *token_id = static_cast<std::int32_t>(best_index);
  return Status::Ok();
}

// 单步生成：调用 TinyLlamaModel forward，然后用 greedy 选出一个 token。
Status GenerateOneStep(const TinyLlamaModel& model,
                       const GeneratorConfig& config,
                       const Tensor& hidden_state,
                       GenerationState* state,
                       std::int32_t* next_token_id) {
  if (state == nullptr) {
    return Status::InvalidArgument("GenerateOneStep state is null");
  }

  if (next_token_id == nullptr) {
    return Status::InvalidArgument("GenerateOneStep next_token_id is null");
  }

  Status status = ValidateGeneratorConfig(config);
  if (!status.ok()) {
    return status;
  }

  if (state->finished) {
    return Status::InvalidArgument("GenerateOneStep state is already finished");
  }

  if (state->token_ids.size() >= config.max_token_count) {
    return Status::InvalidArgument(
        "GenerateOneStep reached max_token_count");
  }

  status = ValidateOneStepHiddenState(model, hidden_state);
  if (!status.ok()) {
    return status;
  }

  Tensor probabilities(DType::kFloat32,
                       TensorShape({1, model.config().vocab_size}));
  status = model.forward(hidden_state, &probabilities);
  if (!status.ok()) {
    return status;
  }

  std::int32_t token_id = -1;
  status = GreedyNextToken(probabilities, &token_id);
  if (!status.ok()) {
    return status;
  }

  state->token_ids.push_back(token_id);
  if (config.stop_on_eos && token_id == config.eos_token_id) {
    state->finished = true;
  }

  *next_token_id = token_id;
  return Status::Ok();
}

}  // namespace firstllm
