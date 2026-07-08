#include "firstllm/model/kv_cache.h"

#include <cstring>
#include <string>
#include <utility>

namespace firstllm {
namespace {

// 检查 KV cache 配置是否足以分配和索引缓存。
Status ValidateConfig(const KvCacheConfig& config) {
  if (config.layer_count == 0) {
    return Status::InvalidArgument("KvCache layer_count must be non-zero");
  }

  if (config.max_token_count == 0) {
    return Status::InvalidArgument(
        "KvCache max_token_count must be non-zero");
  }

  if (config.kv_head_count == 0) {
    return Status::InvalidArgument("KvCache kv_head_count must be non-zero");
  }

  if (config.head_dim == 0) {
    return Status::InvalidArgument("KvCache head_dim must be non-zero");
  }

  return Status::Ok();
}

// 检查 layer_index 是否落在缓存层数范围内。
Status ValidateLayerIndex(const KvCacheConfig& config,
                          std::size_t layer_index) {
  if (layer_index >= config.layer_count) {
    return Status::InvalidArgument("KvCache layer index is out of range");
  }

  return Status::Ok();
}

// 检查单 token key/value tensor 的 dtype 和形状。
Status ValidateStepTensor(const KvCacheConfig& config,
                          const Tensor& tensor,
                          const char* tensor_name) {
  if (tensor.dtype() != DType::kFloat32) {
    return Status::InvalidArgument(
        std::string(tensor_name) + " must be float32");
  }

  if (tensor.shape().rank() != 2) {
    return Status::InvalidArgument(
        std::string(tensor_name) + " must be 2D");
  }

  if (tensor.shape().dim(0) != config.kv_head_count ||
      tensor.shape().dim(1) != config.head_dim) {
    return Status::InvalidArgument(
        std::string(tensor_name) + " shape is invalid");
  }

  return Status::Ok();
}

// 计算缓存内部 [token, head, dim] 的 row-major 线性索引。
std::size_t CacheIndex(const KvCacheConfig& config,
                       std::size_t token_index,
                       std::size_t head_index,
                       std::size_t dim_index) {
  return (token_index * config.kv_head_count + head_index) *
             config.head_dim +
         dim_index;
}

// 从 Tensor raw bytes 中读取 float32。
float ReadFloat(const Tensor& tensor, std::size_t index) {
  float value = 0.0f;
  std::memcpy(&value,
              tensor.data() + index * sizeof(float),
              sizeof(float));
  return value;
}

// 把一帧 [kv_head_count, head_dim] 的 key/value 拷贝到指定 token 位置。
void CopyStepToCache(const KvCacheConfig& config,
                     const Tensor& step,
                     std::size_t token_index,
                     Tensor* cache) {
  const std::size_t element_count = config.kv_head_count * config.head_dim;
  const std::size_t dst_index =
      CacheIndex(config, token_index, 0, 0);

  std::memcpy(cache->data() + dst_index * sizeof(float),
              step.data(),
              element_count * sizeof(float));
}

// 统一读取 key/value 中的一个元素。
Status ReadCacheValue(const KvCacheConfig& config,
                      const std::vector<Tensor>& cache,
                      const std::vector<std::size_t>& token_counts,
                      std::size_t layer_index,
                      std::size_t token_index,
                      std::size_t head_index,
                      std::size_t dim_index,
                      float* value) {
  if (value == nullptr) {
    return Status::InvalidArgument("KvCache output value pointer is null");
  }

  Status status = ValidateConfig(config);
  if (!status.ok()) {
    return status;
  }

  status = ValidateLayerIndex(config, layer_index);
  if (!status.ok()) {
    return status;
  }

  if (token_index >= token_counts[layer_index]) {
    return Status::InvalidArgument("KvCache token index is out of range");
  }

  if (head_index >= config.kv_head_count) {
    return Status::InvalidArgument("KvCache head index is out of range");
  }

  if (dim_index >= config.head_dim) {
    return Status::InvalidArgument("KvCache dim index is out of range");
  }

  const std::size_t index =
      CacheIndex(config, token_index, head_index, dim_index);
  *value = ReadFloat(cache[layer_index], index);
  return Status::Ok();
}

}  // namespace

// 构造时按 layer 预分配 key/value 缓冲区；无效 config 会在公开方法中返回错误。
KvCache::KvCache(KvCacheConfig config)
    : config_(std::move(config)),
      keys_(),
      values_(),
      token_counts_(config_.layer_count, 0) {
  keys_.reserve(config_.layer_count);
  values_.reserve(config_.layer_count);

  for (std::size_t layer = 0; layer < config_.layer_count; ++layer) {
    TensorShape cache_shape({
        config_.max_token_count,
        config_.kv_head_count,
        config_.head_dim,
    });
    keys_.emplace_back(DType::kFloat32, cache_shape);
    values_.emplace_back(DType::kFloat32, cache_shape);
  }
}

// 返回 KV cache 配置。
const KvCacheConfig& KvCache::config() const {
  return config_;
}

// 查询某一层已缓存 token 数。
Status KvCache::token_count(std::size_t layer_index,
                            std::size_t* token_count) const {
  if (token_count == nullptr) {
    return Status::InvalidArgument("KvCache token_count pointer is null");
  }

  Status status = ValidateConfig(config_);
  if (!status.ok()) {
    return status;
  }

  status = ValidateLayerIndex(config_, layer_index);
  if (!status.ok()) {
    return status;
  }

  *token_count = token_counts_[layer_index];
  return Status::Ok();
}

// 向指定层追加一个 token 的 key/value。
Status KvCache::append(std::size_t layer_index,
                       const Tensor& key,
                       const Tensor& value) {
  Status status = ValidateConfig(config_);
  if (!status.ok()) {
    return status;
  }

  status = ValidateLayerIndex(config_, layer_index);
  if (!status.ok()) {
    return status;
  }

  if (token_counts_[layer_index] >= config_.max_token_count) {
    return Status::InvalidArgument("KvCache layer is full");
  }

  status = ValidateStepTensor(config_, key, "KvCache key");
  if (!status.ok()) {
    return status;
  }

  status = ValidateStepTensor(config_, value, "KvCache value");
  if (!status.ok()) {
    return status;
  }

  const std::size_t token_index = token_counts_[layer_index];
  CopyStepToCache(config_, key, token_index, &keys_[layer_index]);
  CopyStepToCache(config_, value, token_index, &values_[layer_index]);

  ++token_counts_[layer_index];
  return Status::Ok();
}

// 读取指定 key 元素。
Status KvCache::read_key(std::size_t layer_index,
                         std::size_t token_index,
                         std::size_t head_index,
                         std::size_t dim_index,
                         float* value) const {
  return ReadCacheValue(config_,
                        keys_,
                        token_counts_,
                        layer_index,
                        token_index,
                        head_index,
                        dim_index,
                        value);
}

// 读取指定 value 元素。
Status KvCache::read_value(std::size_t layer_index,
                           std::size_t token_index,
                           std::size_t head_index,
                           std::size_t dim_index,
                           float* value) const {
  return ReadCacheValue(config_,
                        values_,
                        token_counts_,
                        layer_index,
                        token_index,
                        head_index,
                        dim_index,
                        value);
}

// 清空所有层的已写 token 数。
Status KvCache::clear() {
  Status status = ValidateConfig(config_);
  if (!status.ok()) {
    return status;
  }

  for (std::size_t& token_count : token_counts_) {
    token_count = 0;
  }

  return Status::Ok();
}

}  // namespace firstllm
