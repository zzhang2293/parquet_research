#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <stdexcept>

std::size_t byteAlignedULEB128(const char* addr, unsigned long* ret);

std::size_t readULEB128(const char* addr, unsigned long bits, unsigned long* ret);

std::size_t readZigzag(const char* addr, unsigned long bits, long* ret);

std::size_t readByte(const char* addr, unsigned long bits, unsigned long* ret);

std::size_t readVariableBits(const char* addr, unsigned long bits, unsigned long* ret, unsigned long numBits);

uint32_t computeZigZag(int original);

void populateHeaderHelper(std::vector<uint8_t> &encoded, int header, unsigned long long &encodedIndex,
                          uint32_t maxCuttOffs[4], uint8_t iterations, uint8_t mask, uint8_t incr);

void populateHeader(std::vector<uint8_t> &encoded, int headerValues[4], unsigned long long &encodedIndex,
                    uint32_t maxCuttOffs[4], uint8_t iterations, uint8_t mask, uint8_t incr);

int calulateDeltas(std::vector<long long> &blockDeltas, int* rawData, int &rawIndex, uint8_t blockSize, int minDelta);

void calculateMaxFrameOfReference (uint64_t setIndex, uint8_t miniBlockCount, uint8_t miniBlockSize, uint32_t * maxFrameOfReferences, 
                                   uint8_t * miniBlockBitWidths, std::vector<long long> &blockDeltas, int minDelta);

void encodeByte(uint8_t byteToEncode, uint8_t &encodedBitsRemaining, uint8_t mask, uint8_t incr, std::vector<long long> &blockDeltas,
                unsigned long long &encodedIndex, std::vector<uint8_t> &encoded);

void encodeVariableBits(long long intToEncode, uint8_t &encodedBitsRemaining, uint8_t mask, uint8_t incr,
                        std::vector<long long> &blockDeltas, unsigned long long &encodedIndex, std::vector<uint8_t> &encoded);

std::vector<uint8_t> encode32BitData(int *rawData, uint64_t length);