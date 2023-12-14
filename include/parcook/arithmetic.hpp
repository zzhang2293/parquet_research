#pragma once

#include <cstdint>
#include <parquet/types.h>

namespace parcook {

void add_plain_i32(void *data,
                   int64_t num_values,
                   int32_t value,
                   int32_t *result);

void add_delta_i32(void *data,
                   int64_t num_values,
                   int32_t value,
                   int32_t *result);

/**
 * Add encoded 32-bit integers.
 * @param encoding Parquet encoding.
 * @param data Pointer to the encoded data.
 * @param num_values Number of values.
 * @param value Value to add.
 * @param result Result.
 */
void add_i32(parquet::Encoding::type encoding,
             void *data,
             int64_t num_values,
             int32_t value,
             int32_t *result);

} // namespace parcook
