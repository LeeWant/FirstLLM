#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"
#include "firstllm/model/tiny_llama.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace firstllm {

// GeneratorConfig 描述第 13 章第一版单步生成需要的最小运行时配置。
// 当前只支持 greedy，不包含 temperature、top-k、top-p 或随机采样。
struct GeneratorConfig {
  std::size_t max_token_count = 0;   // 本次生成最多允许保存多少个 token。
  std::int32_t eos_token_id = -1;    // 结束 token；负数表示当前不启用 eos 判断。
  bool stop_on_eos = false;          // 是否在生成 eos_token_id 后标记完成。
};

// GenerationState 保存自回归生成过程中已经得到的 token。
// 当前只保存 token id 和 finished 标记，后续可接入 KV cache、position 和采样状态。
struct GenerationState {
  std::vector<std::int32_t> token_ids;  // 已生成 token id 序列。
  bool finished = false;                // 是否已经遇到终止条件。
};

// 从概率分布中选择最后一行概率最大的 token id。
// probabilities shape 必须是 [tokens, vocab_size]，当前只支持 float32。
Status GreedyNextToken(const Tensor& probabilities,
                       std::int32_t* token_id);

// 执行一步极简自回归生成：
// hidden_state [1, hidden_size] -> TinyLlamaModel::forward -> greedy token。
// 当前暂不根据 token id 查 embedding，也不更新 KV cache；这些会在后续节点扩展。
Status GenerateOneStep(const TinyLlamaModel& model,
                       const GeneratorConfig& config,
                       const Tensor& hidden_state,
                       GenerationState* state,
                       std::int32_t* next_token_id);

}  // namespace firstllm
