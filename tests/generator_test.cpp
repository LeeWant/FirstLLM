#include "firstllm/runtime/generator.h"

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

// 构造一个第 12 章 TinyLlamaModel，用来测试 generator 如何调用 forward。
firstllm::TinyLlamaModel MakeModel(firstllm::Tensor* norm_weight,
                                   firstllm::Tensor* projection,
                                   firstllm::Tensor* bias) {
  WriteFloat(norm_weight, 0, 1.0f);
  WriteFloat(norm_weight, 1, 1.0f);

  // projection 是 2x3。第 1 个 vocab 会得到更高概率，便于 greedy 验证。
  WriteFloat(projection, 0, 1.0f);
  WriteFloat(projection, 1, 0.0f);
  WriteFloat(projection, 2, 0.0f);
  WriteFloat(projection, 3, 0.0f);
  WriteFloat(projection, 4, 1.0f);
  WriteFloat(projection, 5, 1.0f);

  WriteFloat(bias, 0, 0.0f);
  WriteFloat(bias, 1, 0.5f);
  WriteFloat(bias, 2, -0.5f);

  const firstllm::TinyLlamaConfig model_config{
      2,     // hidden_size
      3,     // vocab_size
      0.0f,  // epsilon 设为 0，方便确定 greedy 结果。
  };
  const firstllm::TinyLlamaWeights weights{
      norm_weight,
      projection,
      bias,
  };

  return firstllm::TinyLlamaModel(model_config, weights);
}

}  // namespace

int main() {
  // GreedyNextToken 只读取最后一行概率，第一行最大值不会影响结果。
  firstllm::Tensor probabilities(firstllm::DType::kFloat32,
                                 firstllm::TensorShape({2, 3}));
  WriteFloat(&probabilities, 0, 0.9f);
  WriteFloat(&probabilities, 1, 0.1f);
  WriteFloat(&probabilities, 2, 0.0f);
  WriteFloat(&probabilities, 3, 0.2f);
  WriteFloat(&probabilities, 4, 0.3f);
  WriteFloat(&probabilities, 5, 0.8f);

  std::int32_t token_id = -1;
  firstllm::Status status =
      firstllm::GreedyNextToken(probabilities, &token_id);
  assert(status.ok());
  assert(token_id == 2);

  // 相同概率时保留更小的 token id，让 greedy 结果稳定。
  firstllm::Tensor tie_probabilities(firstllm::DType::kFloat32,
                                     firstllm::TensorShape({1, 3}));
  WriteFloat(&tie_probabilities, 0, 0.7f);
  WriteFloat(&tie_probabilities, 1, 0.7f);
  WriteFloat(&tie_probabilities, 2, 0.1f);

  status = firstllm::GreedyNextToken(tie_probabilities, &token_id);
  assert(status.ok());
  assert(token_id == 0);

  // 错误路径：GreedyNextToken 参数必须是 2D float32，输出指针不能为空。
  const firstllm::Status null_token =
      firstllm::GreedyNextToken(probabilities, nullptr);
  assert(!null_token.ok());
  assert(null_token.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_dtype_probabilities(firstllm::DType::kInt32,
                                             firstllm::TensorShape({1, 3}));
  const firstllm::Status dtype_error =
      firstllm::GreedyNextToken(wrong_dtype_probabilities, &token_id);
  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_rank_probabilities(firstllm::DType::kFloat32,
                                            firstllm::TensorShape({3}));
  const firstllm::Status rank_error =
      firstllm::GreedyNextToken(wrong_rank_probabilities, &token_id);
  assert(!rank_error.ok());
  assert(rank_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 构造 TinyLlamaModel 和单 token hidden state，验证 GenerateOneStep。
  firstllm::Tensor norm_weight(firstllm::DType::kFloat32,
                               firstllm::TensorShape({2}));
  firstllm::Tensor projection(firstllm::DType::kFloat32,
                              firstllm::TensorShape({2, 3}));
  firstllm::Tensor bias(firstllm::DType::kFloat32,
                        firstllm::TensorShape({3}));
  firstllm::TinyLlamaModel model =
      MakeModel(&norm_weight, &projection, &bias);

  firstllm::Tensor hidden_state(firstllm::DType::kFloat32,
                                firstllm::TensorShape({1, 2}));
  WriteFloat(&hidden_state, 0, 3.0f);
  WriteFloat(&hidden_state, 1, 4.0f);

  const firstllm::GeneratorConfig config{
      2,      // max_token_count
      -1,     // eos_token_id 不启用
      false,  // stop_on_eos
  };

  firstllm::GenerationState state;
  status = firstllm::GenerateOneStep(model,
                                     config,
                                     hidden_state,
                                     &state,
                                     &token_id);
  assert(status.ok());
  assert(token_id == 1);
  assert(state.token_ids.size() == 1);
  assert(state.token_ids[0] == 1);
  assert(!state.finished);

  // 第二步仍能追加，达到 max_token_count 后下一次会失败。
  status = firstllm::GenerateOneStep(model,
                                     config,
                                     hidden_state,
                                     &state,
                                     &token_id);
  assert(status.ok());
  assert(state.token_ids.size() == 2);

  const firstllm::Status max_error =
      firstllm::GenerateOneStep(model,
                                config,
                                hidden_state,
                                &state,
                                &token_id);
  assert(!max_error.ok());
  assert(max_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // stop_on_eos 启用时，生成 eos 后会把 state 标记为 finished。
  const firstllm::GeneratorConfig eos_config{
      3,
      1,
      true,
  };
  firstllm::GenerationState eos_state;
  status = firstllm::GenerateOneStep(model,
                                     eos_config,
                                     hidden_state,
                                     &eos_state,
                                     &token_id);
  assert(status.ok());
  assert(token_id == 1);
  assert(eos_state.finished);

  const firstllm::Status finished_error =
      firstllm::GenerateOneStep(model,
                                eos_config,
                                hidden_state,
                                &eos_state,
                                &token_id);
  assert(!finished_error.ok());
  assert(finished_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：GenerateOneStep 的 state 和 next_token_id 不能为空。
  firstllm::GenerationState empty_state;
  const firstllm::Status null_state =
      firstllm::GenerateOneStep(model,
                                config,
                                hidden_state,
                                nullptr,
                                &token_id);
  assert(!null_state.ok());
  assert(null_state.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status null_next =
      firstllm::GenerateOneStep(model,
                                config,
                                hidden_state,
                                &empty_state,
                                nullptr);
  assert(!null_next.ok());
  assert(null_next.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：max_token_count 不能为 0，eos 配置也要自洽。
  const firstllm::GeneratorConfig bad_max_config{
      0,
      -1,
      false,
  };
  const firstllm::Status bad_max =
      firstllm::GenerateOneStep(model,
                                bad_max_config,
                                hidden_state,
                                &empty_state,
                                &token_id);
  assert(!bad_max.ok());
  assert(bad_max.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::GeneratorConfig bad_eos_config{
      1,
      -1,
      true,
  };
  const firstllm::Status bad_eos =
      firstllm::GenerateOneStep(model,
                                bad_eos_config,
                                hidden_state,
                                &empty_state,
                                &token_id);
  assert(!bad_eos.ok());
  assert(bad_eos.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：单步生成只接受 [1, hidden_size] 的 float32 hidden_state。
  firstllm::Tensor wrong_dtype_hidden(firstllm::DType::kInt32,
                                      firstllm::TensorShape({1, 2}));
  const firstllm::Status hidden_dtype =
      firstllm::GenerateOneStep(model,
                                config,
                                wrong_dtype_hidden,
                                &empty_state,
                                &token_id);
  assert(!hidden_dtype.ok());
  assert(hidden_dtype.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_token_count(firstllm::DType::kFloat32,
                                     firstllm::TensorShape({2, 2}));
  const firstllm::Status token_count_error =
      firstllm::GenerateOneStep(model,
                                config,
                                wrong_token_count,
                                &empty_state,
                                &token_id);
  assert(!token_count_error.ok());
  assert(token_count_error.code() == firstllm::ErrorCode::kInvalidArgument);

  firstllm::Tensor wrong_hidden_size(firstllm::DType::kFloat32,
                                     firstllm::TensorShape({1, 3}));
  const firstllm::Status hidden_size_error =
      firstllm::GenerateOneStep(model,
                                config,
                                wrong_hidden_size,
                                &empty_state,
                                &token_id);
  assert(!hidden_size_error.ok());
  assert(hidden_size_error.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "generator_test passed\n";
  return 0;
}
