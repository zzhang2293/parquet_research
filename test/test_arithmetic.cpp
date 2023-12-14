#include "test.hpp"

#include "parcook/arithmetic.hpp"

#include <filesystem>

std::string filename = "test_arithmetic.parquet";

void test_arithmetic_add_i32(parquet::Encoding::type encoding) {
  arrow::Int32Builder builder;
  
  int MAX_VALUE_LIMIT = 1000; // change this number to change the max limit of the total value count
  // ARROW_ABORT_NOT_OK(builder.AppendValues({1005, 1015, 1053, 1069, 1080, 1099})); //accending positive => mindelta: 10
  // ARROW_ABORT_NOT_OK(builder.AppendValues({10, 5, 3, 1})); //decending positive => mindelta: -5
  // ARROW_ABORT_NOT_OK(builder.AppendValues({-15, -10, -7, -5})); //accending negative => mindelta: -2
  // ARROW_ABORT_NOT_OK(builder.AppendValues({-5, -10, -15, - 17, -25})); // decending negative => mindelta: -8

  // ARROW_ABORT_NOT_OK(builder.AppendValues({5,50, 75, 115, 165, 240, 250, 300, 400, 575})); //mindelta: 10

  ARROW_ABORT_NOT_OK(builder.AppendValues({7, 5, 3, 1, 2, 3, 4, 5}));

  // ARROW_ABORT_NOT_OK(builder.AppendValues({-15, -8, 9, 26, 56})); mindelta: -7

  // ARROW_ABORT_NOT_OK(builder.AppendValues({25, 15, 10, 5, 3, 1, -20, 50, 234, 2345, 0})); //multiple => mindelta: 70

  // ARROW_ABORT_NOT_OK(builder.AppendValues({25, 15, 10, 5, 3, 1, 20, 50, 234, 2345, 0})); //multiple => mindelta: 19
  // ARROW_ABORT_NOT_OK(builder.AppendValues({25, 15, 10, 5, 3, 1, 19, 50, 234, 2345, 0})); //multiple => mindelta: 18?

  // In terms of positive numbers, the mindelta appears to be working only with numbers that are accending or getting larger
  // the mindelta won't seem to regester for a negative mindelta if a positive one exists in the sequence

  std::shared_ptr<arrow::Array> array;
  ARROW_ABORT_NOT_OK(builder.Finish(&array));

  parquet_write(filename, {array}, {encoding});

  int32_t value = 3;
  std::vector<int32_t> result(MAX_VALUE_LIMIT);

  parquet_read(filename, 0, [&](void *data, int64_t num_values) {
    parcook::add_i32(encoding, data, num_values, value, result.data());
  });

  TEST_ASSERT((result == std::vector<int32_t>{4, 5, 6, 7, 8, 9, 10, 11, 12, 13}));

  std::filesystem::remove(filename);
}

void test_arithmetic_add_plain_i32() {
  test_arithmetic_add_i32(parquet::Encoding::PLAIN);
}

void test_arithmetic_add_delta_i32() {
  test_arithmetic_add_i32(parquet::Encoding::DELTA_BINARY_PACKED);
}

int main() {
  TEST_RUN(test_arithmetic_add_plain_i32);

  // TODO: Uncomment when implemented.
  TEST_RUN(test_arithmetic_add_delta_i32);

  return TEST_EXIT_CODE;
}
