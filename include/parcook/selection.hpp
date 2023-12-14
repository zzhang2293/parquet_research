#pragma once

#include <cstdint>
#include <parquet/types.h>

namespace parcook {

void cmp_lt_plain_i32(void *data,
                      int64_t num_values,
                      int32_t value,
                      uint64_t *result);

void cmp_lt_delta_i32(void *data,
                      int64_t num_values,
                      int32_t value,
                      uint64_t *result);

/**
 * Compare encoded 32-bit integers.
 * @param encoding Parquet encoding.
 * @param data Pointer to the encoded data.
 * @param num_values Number of values.
 * @param value Value to compare against.
 * @param result Bit-packed result.
 */
void cmp_lt_i32(parquet::Encoding::type encoding,
                void *data,
                int64_t num_values,
                int32_t value,
                uint64_t *result);

} // namespace parcook
