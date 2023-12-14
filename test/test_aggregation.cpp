#include "test.hpp"

#include "parcook/aggregation.hpp"

#include <filesystem>

std::string filename = "test_aggregation.parquet";

void test_aggregation_sum_i32(parquet::Encoding::type encoding) {
  arrow::Int32Builder builder;
  ARROW_ABORT_NOT_OK(builder.AppendValues({1, 2, 3, 4, 5}));

  std::shared_ptr<arrow::Array> array;
  ARROW_ABORT_NOT_OK(builder.Finish(&array));

  parquet_write(filename, {array}, {encoding});

  int32_t result = 0;

  parquet_read(filename, 0, [&](void *data, int64_t num_values) {
    parcook::sum_i32(encoding, data, num_values, result);
  });

  TEST_ASSERT(result == 15);

  std::filesystem::remove(filename);
}

void test_aggregation_sum_plain_i32() {
  test_aggregation_sum_i32(parquet::Encoding::PLAIN);
}

void test_aggregation_sum_delta_i32() {
  test_aggregation_sum_i32(parquet::Encoding::DELTA_BINARY_PACKED);
}

int main() {
  TEST_RUN(test_aggregation_sum_plain_i32);

  // TODO: Uncomment when implemented.
  TEST_RUN(test_aggregation_sum_delta_i32);

  return TEST_EXIT_CODE;
}
