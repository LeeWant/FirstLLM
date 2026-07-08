#include "firstllm/model/kv_cache.h"

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

// 创建一帧 [kv_head_count, head_dim] 的测试 key/value。
firstllm::Tensor MakeStepTensor(float base) {
  firstllm::Tensor tensor(firstllm::DType::kFloat32,
                          firstllm::TensorShape({2, 2}));

  WriteFloat(&tensor, 0, base + 0.0f);
  WriteFloat(&tensor, 1, base + 1.0f);
  WriteFloat(&tensor, 2, base + 2.0f);
  WriteFloat(&tensor, 3, base + 3.0f);

  return tensor;
}

}  // namespace

int main() {
  const firstllm::KvCacheConfig config{
      2,  // layer_count
      2,  // max_token_count
      2,  // kv_head_count
      2,  // head_dim
  };

  firstllm::KvCache cache(config);

  assert(cache.config().layer_count == 2);
  assert(cache.config().max_token_count == 2);
  assert(cache.config().kv_head_count == 2);
  assert(cache.config().head_dim == 2);

  std::size_t count = 99;
  firstllm::Status status = cache.token_count(0, &count);
  assert(status.ok());
  assert(count == 0);

  firstllm::Tensor key0 = MakeStepTensor(1.0f);
  firstllm::Tensor value0 = MakeStepTensor(10.0f);

  status = cache.append(0, key0, value0);
  assert(status.ok());

  status = cache.token_count(0, &count);
  assert(status.ok());
  assert(count == 1);

  float value = 0.0f;
  status = cache.read_key(0, 0, 1, 0, &value);
  assert(status.ok());
  assert(value == 3.0f);

  status = cache.read_value(0, 0, 1, 1, &value);
  assert(status.ok());
  assert(value == 13.0f);

  firstllm::Tensor key1 = MakeStepTensor(20.0f);
  firstllm::Tensor value1 = MakeStepTensor(30.0f);

  status = cache.append(0, key1, value1);
  assert(status.ok());

  status = cache.token_count(0, &count);
  assert(status.ok());
  assert(count == 2);

  status = cache.read_key(0, 1, 0, 1, &value);
  assert(status.ok());
  assert(value == 21.0f);

  // layer 1 独立计数，模拟不同层各自追加 KV。
  status = cache.append(1, key0, value0);
  assert(status.ok());

  status = cache.token_count(1, &count);
  assert(status.ok());
  assert(count == 1);

  status = cache.read_value(1, 0, 0, 0, &value);
  assert(status.ok());
  assert(value == 10.0f);

  // 错误路径：layer 0 容量已满，不能继续追加。
  const firstllm::Status full_error = cache.append(0, key0, value0);
  assert(!full_error.ok());
  assert(full_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：读取未写入的 token。
  const firstllm::Status token_error = cache.read_key(1, 1, 0, 0, &value);
  assert(!token_error.ok());
  assert(token_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：layer/head/dim 越界和 null 输出指针。
  const firstllm::Status layer_error = cache.read_key(2, 0, 0, 0, &value);
  assert(!layer_error.ok());
  assert(layer_error.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status head_error = cache.read_key(0, 0, 2, 0, &value);
  assert(!head_error.ok());
  assert(head_error.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status dim_error = cache.read_value(0, 0, 0, 2, &value);
  assert(!dim_error.ok());
  assert(dim_error.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status null_read = cache.read_key(0, 0, 0, 0, nullptr);
  assert(!null_read.ok());
  assert(null_read.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：append 的 key/value 必须是 float32 且 shape 为 [kv_head_count, head_dim]。
  firstllm::Tensor wrong_dtype(firstllm::DType::kInt32,
                               firstllm::TensorShape({2, 2}));
  firstllm::Tensor wrong_rank(firstllm::DType::kFloat32,
                              firstllm::TensorShape({1, 2, 2}));
  firstllm::Tensor wrong_shape(firstllm::DType::kFloat32,
                               firstllm::TensorShape({2, 3}));

  firstllm::KvCache shape_cache(config);

  const firstllm::Status dtype_error =
      shape_cache.append(0, wrong_dtype, value0);
  assert(!dtype_error.ok());
  assert(dtype_error.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status rank_error =
      shape_cache.append(0, wrong_rank, value0);
  assert(!rank_error.ok());
  assert(rank_error.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status shape_error =
      shape_cache.append(0, wrong_shape, value0);
  assert(!shape_error.ok());
  assert(shape_error.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status value_shape_error =
      shape_cache.append(0, key0, wrong_shape);
  assert(!value_shape_error.ok());
  assert(value_shape_error.code() == firstllm::ErrorCode::kInvalidArgument);

  // clear 只重置计数，不重新分配底层内存。
  status = cache.clear();
  assert(status.ok());

  status = cache.token_count(0, &count);
  assert(status.ok());
  assert(count == 0);

  const firstllm::Status read_after_clear =
      cache.read_key(0, 0, 0, 0, &value);
  assert(!read_after_clear.ok());
  assert(read_after_clear.code() == firstllm::ErrorCode::kInvalidArgument);

  // 错误路径：无效配置会在公开方法中返回错误。
  const firstllm::KvCacheConfig bad_config{
      0,
      1,
      1,
      1,
  };
  firstllm::KvCache bad_cache(bad_config);

  const firstllm::Status bad_config_append =
      bad_cache.append(0, key0, value0);
  assert(!bad_config_append.ok());
  assert(bad_config_append.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status bad_config_count =
      bad_cache.token_count(0, &count);
  assert(!bad_config_count.ok());
  assert(bad_config_count.code() == firstllm::ErrorCode::kInvalidArgument);

  const firstllm::Status null_count = cache.token_count(0, nullptr);
  assert(!null_count.ok());
  assert(null_count.code() == firstllm::ErrorCode::kInvalidArgument);

  std::cout << "kv_cache_test passed\n";
  return 0;
}
