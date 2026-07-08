#pragma once

#include "firstllm/core/status.h"
#include "firstllm/core/tensor.h"

#include <cstddef>
#include <vector>

namespace firstllm {

// KvCacheConfig 描述第 13 章最小 KV cache 的固定容量和形状。
// 当前只支持 float32 host tensor，不包含分页缓存、batch 或 device memory。
struct KvCacheConfig {
  std::size_t layer_count = 0;      // Transformer 层数。
  std::size_t max_token_count = 0;  // 每层最多缓存多少个 token。
  std::size_t kv_head_count = 0;    // KV head 数量。
  std::size_t head_dim = 0;         // 每个 head 的维度。
};

// KvCache 保存自回归生成过程中逐 token 追加的 key/value。
// 当前每层独立计数，调用方负责按模型层顺序追加；后续 attention 会按层读取历史 KV。
class KvCache {
 public:
  // 根据 config 预分配每层 key/value 缓冲区。
  explicit KvCache(KvCacheConfig config);

  // 返回 KV cache 配置。
  const KvCacheConfig& config() const;

  // 查询某一层当前已经缓存的 token 数。
  Status token_count(std::size_t layer_index,
                     std::size_t* token_count) const;

  // 向指定层追加一个 token 的 key/value。
  // key/value shape 必须是 [kv_head_count, head_dim]。
  Status append(std::size_t layer_index,
                const Tensor& key,
                const Tensor& value);

  // 读取缓存中的单个 key 元素，用于测试和后续 attention 校验。
  Status read_key(std::size_t layer_index,
                  std::size_t token_index,
                  std::size_t head_index,
                  std::size_t dim_index,
                  float* value) const;

  // 读取缓存中的单个 value 元素，用于测试和后续 attention 校验。
  Status read_value(std::size_t layer_index,
                    std::size_t token_index,
                    std::size_t head_index,
                    std::size_t dim_index,
                    float* value) const;

  // 清空所有层的 token 计数；底层内存保留，后续 append 会覆盖旧数据。
  Status clear();

 private:
  KvCacheConfig config_;                 // 固定容量和形状配置。
  std::vector<Tensor> keys_;             // 每层 key cache，shape: [max_token_count, kv_head_count, head_dim]。
  std::vector<Tensor> values_;           // 每层 value cache，shape 同 keys_。
  std::vector<std::size_t> token_counts_; // 每层已经写入的 token 数。
};

}  // namespace firstllm
