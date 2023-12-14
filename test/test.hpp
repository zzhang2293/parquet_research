#pragma once

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/column_page.h>
#include <parquet/column_reader.h>

#include <iostream>
#include <sstream>

static int test_status = 0;
static int test_exit_code = 0;
std::string test_message;

std::ostringstream &test_message_stream() {
  static std::ostringstream s;
  return s;
}

#define TEST_EXIT_CODE test_exit_code

#define TEST_ASSERT(condition)                                                 \
  do {                                                                         \
    if (!(condition)) {                                                        \
      test_status = 1;                                                         \
      test_exit_code = 1;                                                      \
      std::ostringstream s;                                                    \
      s << __FILE__ << ':' << __LINE__ << ": assertion failed";                \
      test_message = s.str();                                                  \
      return;                                                                  \
    }                                                                          \
  } while (false)

#define TEST_RUN(test)                                                         \
  do {                                                                         \
    test_status = 0;                                                           \
    test();                                                                    \
    if (test_status) {                                                         \
      std::cout << #test << " failed." << std::endl                            \
                << '\t' << test_message << std::endl;                          \
    } else {                                                                   \
      std::cout << #test << " passed." << std::endl;                           \
    }                                                                          \
  } while (0)

#define ARROW_ABORT_NOT_OK(expr)                                               \
  do {                                                                         \
    ::arrow::Status _s = (expr);                                               \
    if (!_s.ok()) {                                                            \
      _s.Abort();                                                              \
    }                                                                          \
  } while (false)

void parquet_write(const std::string &filename,
                   const std::vector<std::shared_ptr<arrow::Array>> &arrays,
                   const std::vector<parquet::Encoding::type> &encodings,
                   int64_t chunk_size = 16 * 1024 * 1024) {
  arrow::SchemaBuilder schema_builder;
  parquet::WriterProperties::Builder writer_properties_builder;

  writer_properties_builder.disable_dictionary();

  for (size_t i = 0; i < arrays.size(); ++i) {
    std::string name = std::to_string(i);

    ARROW_ABORT_NOT_OK(
        schema_builder.AddField(arrow::field(name, arrays[i]->type(), false)));

    writer_properties_builder.encoding(name, encodings[i]);
  }

  std::shared_ptr<arrow::Table> table =
      arrow::Table::Make(schema_builder.Finish().ValueOrDie(), arrays);

  std::shared_ptr<arrow::io::FileOutputStream> file =
      arrow::io::FileOutputStream::Open(filename).ValueOrDie();

  std::shared_ptr<parquet::WriterProperties> writer_properties =
      writer_properties_builder.build();

  ARROW_ABORT_NOT_OK(parquet::arrow::WriteTable(*table,
                                                arrow::default_memory_pool(),
                                                file,
                                                chunk_size,
                                                writer_properties));

  ARROW_ABORT_NOT_OK(file->Close());
}

template <typename F>
void parquet_read(const std::string &filename, int column, F &&callback) {
  std::shared_ptr<arrow::io::ReadableFile> file =
      arrow::io::ReadableFile::Open(filename).ValueOrDie();

  std::unique_ptr<parquet::arrow::FileReader> file_reader;
  ARROW_ABORT_NOT_OK(parquet::arrow::OpenFile(
      file, arrow::default_memory_pool(), &file_reader));

  for (int i = 0; i < file_reader->num_row_groups(); ++i) {
    std::shared_ptr<parquet::RowGroupReader> row_group =
        file_reader->parquet_reader()->RowGroup(i);

    std::unique_ptr<parquet::PageReader> page_reader =
        row_group->GetColumnPageReader(column);

    while (std::shared_ptr<parquet::Page> page = page_reader->NextPage()) {
      auto data_page = std::static_pointer_cast<parquet::DataPage>(page);
      callback((void *)data_page->data(), data_page->num_values());
    }
  }
}

arrow::Status parquet_read2(){
  std::string filename = "test_selection.parquet";
  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::io::RandomAccessFile> input;
  ARROW_ASSIGN_OR_RAISE(input, arrow::io::ReadableFile::Open(filename));
  std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
  ARROW_RETURN_NOT_OK(parquet::arrow::OpenFile(input, pool, &arrow_reader));

  std::shared_ptr<arrow::Table> table;
  ARROW_RETURN_NOT_OK(arrow_reader->ReadTable(&table));
  std::cout << "number of rows in the table is: "<< table->num_rows() << std::endl;
  // show the first column value
  std::shared_ptr<arrow::ChunkedArray> chunked_array = table->column(0);
  std::shared_ptr<arrow::Array> array = chunked_array->chunk(0);
  std::cout << "chuncked array length is " << chunked_array->length() << std::endl;
  std::cout << "array length is " << array->length() << std::endl;
  std::cout << "array value is " << array->ToString() << std::endl;

  return arrow::Status::OK();
  }


