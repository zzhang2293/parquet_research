#include "parcook/selection.hpp"

#include <stdexcept>
#include "common.h"

void parcook::cmp_lt_plain_i32(void *data,
                               int64_t num_values,
                               int32_t value,
                               uint64_t *result) {
  auto *values = (int32_t *)data;

  for (int64_t i = 0; i < num_values; ++i) {
    result[i / 64] |= (uint64_t)(values[i] < value) << i % 64;
  }
}

void parcook::cmp_lt_delta_i32(void *data,
                               int64_t num_values,
                               int32_t value,
                               uint64_t *result) {
    auto block_size = 0ul;
    auto miniblock_count = 0ul;
    auto total_value_count = 0ul;
    auto first_value = 0l;
    auto sumBitsConsumed = 0ul;
    auto encoded_data = static_cast<char*> (data);

    sumBitsConsumed += readULEB128(encoded_data, sumBitsConsumed, &block_size);
    sumBitsConsumed += readULEB128(encoded_data, sumBitsConsumed, &miniblock_count);
    sumBitsConsumed += readULEB128(encoded_data, sumBitsConsumed, &total_value_count);
    sumBitsConsumed += readZigzag(encoded_data, sumBitsConsumed, &first_value);

    std::cout <<  "block_size: " << block_size <<std::endl;
    std::cout << "miniblock_count: " << miniblock_count <<std::endl;
    std::cout << "total_value_count: " << total_value_count <<std::endl;
    std::cout << "First Value: " << first_value << std::endl;
}

void parcook::cmp_lt_i32(parquet::Encoding::type encoding,
                         void *data,
                         int64_t num_values,
                         int32_t value,
                         uint64_t *result) {
  switch (encoding) {
  case parquet::Encoding::PLAIN:
    cmp_lt_plain_i32(data, num_values, value, result);
    break;
  case parquet::Encoding::DELTA_BINARY_PACKED:
    cmp_lt_delta_i32(data, num_values, value, result);
    break;
  default:
    throw std::runtime_error("unsupported encoding");
  }
}