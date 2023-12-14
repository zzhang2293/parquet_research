#include "test.hpp"

#include "parcook/selection.hpp"

#include <filesystem>

std::string filename = "test_selection.parquet";

void test_selection_cmp_lt_i32(parquet::Encoding::type encoding) {
  arrow::Int32Builder builder;
  ARROW_ABORT_NOT_OK(builder.AppendValues({1, 2, 3, 4, 5}));

  std::shared_ptr<arrow::Array> array;
  ARROW_ABORT_NOT_OK(builder.Finish(&array));

  parquet_write(filename, {array}, {encoding});

  int32_t value = 3;
  uint64_t result = 0;

  parquet_read(filename, 0, [&](void *data, int64_t num_values) {
    parcook::cmp_lt_i32(encoding, data, num_values, value, &result);
  });

  TEST_ASSERT(result == 0b00011);

  std::filesystem::remove(filename);
}

void test_selection_cmp_lt_plain_i32() {
  test_selection_cmp_lt_i32(parquet::Encoding::PLAIN);
}

void test_selection_cmp_lt_delta_i32() {
  test_selection_cmp_lt_i32(parquet::Encoding::DELTA_BINARY_PACKED);
}

int main() {
  TEST_RUN(test_selection_cmp_lt_plain_i32);

  // TODO: Uncomment when implemented.
  TEST_RUN(test_selection_cmp_lt_delta_i32);

  return TEST_EXIT_CODE;
}
