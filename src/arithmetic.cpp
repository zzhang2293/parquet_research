#include "parcook/arithmetic.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "common.h"

void parcook::add_plain_i32(void *data,
                            int64_t num_values,
                            int32_t value,
                            int32_t *result) {
  auto *values = (int32_t *)data;

  for (int64_t i = 0; i < num_values; ++i) {
    result[i] = values[i] + value;
  }
}

void parcook::add_delta_i32(void *data,
                            int64_t num_values,
                            int32_t value,
                            int32_t *result) {

  //hand encoded

  //128 uleb
  int8_t a = 128;
  int8_t b = 1;

  //4 uleb
  int8_t c = 4;

  //6 uleb
  int8_t d = 6;

  //877 zigzag
  int8_t e = 218;
  int8_t f = 13;


  std::vector<int8_t> vec;
  vec.push_back(a);
  vec.push_back(b);
  vec.push_back(c);
  vec.push_back(d);
  vec.push_back(e);
  vec.push_back(f);
  
  int8_t *array = &vec[0];

  void *voidPointer = array;
  auto encoded_data = static_cast<char *>(voidPointer);

  unsigned long first = 0;
  unsigned long second = 0;
  unsigned long third = 0;
  long fourth = 0;
  long fifth = 0;
  unsigned long sixth = 0;

  int numBits = 0;
  numBits += readULEB128(encoded_data, 0, &first);
  numBits += readULEB128(encoded_data, numBits, &second);
  numBits += readULEB128(encoded_data, numBits, &third);
  numBits += readZigzag(encoded_data, numBits, &fourth);
  
  // printf("%li\n", first);
  // printf("%li\n", second);
  // printf("%li\n", third);
  // printf("%li\n", fourth); 

  //now using the encoder
  int myLength = 6;
  int numbers[myLength] = {877, 328, 953, 466, 26, 75}; //deltas: -549,  625, -487, -440,  49 (mindelta:  -549)
  int *pointer = numbers;                    //deltas - mindelta:    0, 1174,    62, 109, 598 (maxbitwidth: 11)

  for (int i = 0; i < myLength; i++) {
    printf("%i ", numbers[i]);
  } printf("\n");
  // printf("\nminDelta should be: -549\n");

  std::vector<uint8_t> encodedVector = encode32BitData(pointer, myLength);

  uint8_t *encodedArray = &encodedVector[0];

  void *vptr = encodedArray;
  auto encoded_data_new = static_cast<char*>(vptr);

  unsigned long firstEncoded = 0;
  unsigned long secondEncoded = 0;
  unsigned long thirdEncoded = 0;
  long fourthEncoded = 0;
  long five = 0;
  unsigned long six = 0;
  unsigned long seven = 0;
  unsigned long eight = 0;
  unsigned long nine = 0;

  unsigned long ten = 0;
  unsigned long eleven = 0;
  unsigned long twelve = 0;
  unsigned long thirteen = 0;
  unsigned long fourteen = 0;

  int newBits = 0;
  newBits += readULEB128(encoded_data_new, newBits, &firstEncoded);
  newBits += readULEB128(encoded_data_new, newBits, &secondEncoded);
  newBits += readULEB128(encoded_data_new, newBits, &thirdEncoded);
  newBits += readZigzag(encoded_data_new, newBits, &fourthEncoded);

  newBits += readZigzag(encoded_data_new, newBits, &five);

  newBits += readByte(encoded_data_new, newBits, &six);
  newBits += readByte(encoded_data_new, newBits, &seven);
  newBits += readByte(encoded_data_new, newBits, &eight);
  newBits += readByte(encoded_data_new, newBits, &nine);

  // newBits += readVariableBits(encoded_data_new, six, &ten, newBits);
  // newBits += readVariableBits(encoded_data_new, six, &eleven, newBits);
  // newBits += readVariableBits(encoded_data_new, six, &twelve, newBits);
  // newBits += readVariableBits(encoded_data_new, six, &thirteen, newBits);
  // newBits += readVariableBits(encoded_data_new, six, &fourteen, newBits);

  printf("\nheader:\n");
  printf("want: %li, have: %li\n", first, firstEncoded);
  printf("want: %li, have: %li\n", second, secondEncoded);
  printf("want: %li, have: %li\n", third, thirdEncoded);
  printf("want: %li, have: %li\n", fourth, fourthEncoded);
  printf("\n");
  printf("block: mindelta, four single byte headers, miniblockvalues\n");
  printf("want: -549, have: %li\n", five);
  printf("\n");
  printf("want: 11, have: %li\n", six);
  printf("want: 0,  have: %li\n", seven);
  printf("want: 0,  have: %li\n", eight);
  printf("want: 0,  have: %li\n", nine);
  printf("\n");
  // printf("want: 0,    have: %li\n", ten);
  // printf("want: 1174, have: %li\n", eleven);
  // printf("want: 62,   have: %li\n", twelve);
  // printf("want: 109,  have: %li\n", thirteen);
  // printf("want: 598,  have: %li\n", fourteen);
}

void parcook::add_i32(parquet::Encoding::type encoding,
                      void *data,
                      int64_t num_values,
                      int32_t value,
                      int32_t *result) {
  switch (encoding) {
  case parquet::Encoding::PLAIN:
    add_plain_i32(data, num_values, value, result);
    break;
  case parquet::Encoding::DELTA_BINARY_PACKED:
    add_delta_i32(data, num_values, value, result);
    break;
  default:
    throw std::runtime_error("unsupported encoding");
  }
}