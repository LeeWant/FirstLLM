#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

#include <cstddef>

namespace firstllm {

// TinyLlamaConfig 描述第 12 章极简 forward 需要的最小模型形状。
// 当前只保留 hidden_size、vocab_size 和 RMSNorm epsilon，不包含层数、head 数或 RoPE 参数。
struct TinyLlamaConfig {
  std::size_t hidden_size = 0;       // hidden 向量宽度。
  std::size_t vocab_size = 0;        // 输出概率分布的宽度。
  float rms_norm_epsilon = 1e-6f;    // RMSNorm 防止除零的稳定项。
};

// TinyLlamaWeights 只引用外部权重 Tensor，不拥有权重内存。
// 当前权重全部要求是 host float32，后续接入 GGUF/量化权重时再扩展所有权和 dtype。
struct TinyLlamaWeights {
  const Tensor* final_norm_weight = nullptr;   // shape: [hidden_size]。
  const Tensor* output_projection = nullptr;   // shape: [hidden_size, vocab_size]。
  const Tensor* output_bias = nullptr;         // shape: [vocab_size]，用于验证 CpuAdd 串联。
};

// TinyLlamaModel 是模型层的第一条最小 forward 路径。
// 它复用已有 CPU kernel，暂不实现 attention、MLP、RoPE、KV cache 或 tokenizer。
class TinyLlamaModel {
 public:
  // 保存 config 和权重引用；真正的形状检查发生在 forward()。
  TinyLlamaModel(TinyLlamaConfig config, TinyLlamaWeights weights);

  // 返回模型配置。
  const TinyLlamaConfig& config() const;

  // 返回权重引用集合。
  const TinyLlamaWeights& weights() const;

  // 执行极简 forward：
  // hidden_states [tokens, hidden_size]
  // -> RMSNorm -> output_projection -> output_bias -> softmax
  // probabilities [tokens, vocab_size]
  Status forward(const Tensor& hidden_states, Tensor* probabilities) const;

 private:
  TinyLlamaConfig config_;    // 极简模型配置。
  TinyLlamaWeights weights_;  // 外部权重引用；不拥有 Tensor。
};

}  // namespace firstllm
