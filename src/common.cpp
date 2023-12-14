#include "common.h"
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <typeinfo>
#include <vector>

std::size_t byteAlignedULEB128(const char* addr, unsigned long* ret) {
  unsigned long result = 0;
  int shift = 0;
  std::size_t count = 0;

  while (1) {
    unsigned char byte = *reinterpret_cast<const unsigned char*>(addr);
    addr++;
    count++;
    result |= (byte & 0x7f) << shift;
    shift += 7;
    if (!(byte & 0x80)) break;
  }

  *ret = result;
  return count * 8;
}

std::size_t readULEB128(const char* addr, unsigned long bits, unsigned long* ret) {
  unsigned long bytes = (bits / 8);
  addr = (addr + bytes);

  if (bits % 8 == 0) {
    return byteAlignedULEB128(addr, ret);
  }

  unsigned long result = 0;
  int shift = 0;
  std::size_t count = 0;
  int offset = bits % 8;

  while(1) {
    unsigned char first  = *reinterpret_cast<const unsigned char*>(addr++);
    unsigned char second = *reinterpret_cast<const unsigned char*>(addr);
    count++;
    first = first >> offset;
    second = second >> (8 - offset);
    unsigned char byte = first + second;
    result |= (byte & 0x7f) << shift;
    shift += 7;
    if (!(byte & 0x80)) break;
  }

  *ret = result;
  return count * 8;
}

std::size_t readZigzag(const char* addr, unsigned long bits, long* ret) {
  auto uleb128 = 0ul;
  auto read = readULEB128(addr, bits, &uleb128);
  *ret = (uleb128 >> 1) ^ -(uleb128 & 1);
  return read;
}

std::size_t readByte(const char* addr, unsigned long bits, unsigned long* ret) {
  unsigned long bytes = (bits / 8);
  addr = (addr + bytes);
  unsigned char byte = *reinterpret_cast<const unsigned char*>(addr);

  if (bits % 8 == 0) {
    *ret = byte;
    return 8;
  }

  addr++;
  int offset = bits % 8;
  unsigned char byte2 = *reinterpret_cast<const unsigned char*>(addr);
  byte = byte >> offset;
  byte2 = byte2 >> (8 - offset);
  *ret = byte + byte2;
  return 8;
}

//needs more work. for reading the bit packed ints that contain the miniblock info. only works for 16 bits or less.
//should probably convert to 64 bit. can check with numBits to determine what casting to use. TODO
std::size_t readVariableBits(const char* addr, unsigned long bits, unsigned long* ret, unsigned long numBits) {
  unsigned long bytes = (bits / 8);
  addr = (addr + bytes);
  unsigned long result = 0;
  int shift = 0;
  int8_t offset = (bits % 8);

  while (shift < numBits) {
    unsigned char first =  *reinterpret_cast<const unsigned char*>(addr++);
    unsigned char second = *reinterpret_cast<const unsigned char*>(addr);
    first = first >> offset;
    second = second >> (8 - offset);
    unsigned char byte = first + second;
    result |= byte << shift;
    shift += 8;
  }

  // result = result >> shift - numBits;
  *ret = result;
  return numBits;
}

uint32_t computeZigZag(int original) {
  if (original > 0) return 2 * original;
  if (original < 0) return 2 * abs(original) - 1;
  return 0;
}

void populateHeaderHelper(std::vector<uint8_t> &encoded, long long header, unsigned long long &encodedIndex,
                          uint32_t maxCuttOffs[4], uint8_t iterations, uint8_t mask, uint8_t incr) {
  for (int index = 0; index < iterations; index++) {
    encoded.push_back((unsigned char)(header >> (index * 7)) & mask);
    encodedIndex++;
    if (header < maxCuttOffs[index]) {
      break;
    }
      encoded[encodedIndex - 1] |= incr;
  }
}

void populateHeader(std::vector<uint8_t> &encoded, long long headerValues[4], unsigned long long &encodedIndex,
                    uint32_t maxCuttOffs[4], uint8_t iterations, uint8_t mask, uint8_t incr) {
  for (int ulebs = 0; ulebs < 3; ulebs++) {
    populateHeaderHelper(encoded, headerValues[ulebs], encodedIndex, maxCuttOffs, iterations, mask, incr);
  }
  long long zigZag = computeZigZag(headerValues[3]);
  populateHeaderHelper(encoded, zigZag, encodedIndex, maxCuttOffs, iterations, mask, incr);
}

//returns the minDelta of the block. computes the blockDeltas and increments rawIndex
int calulateDeltas(std::vector<long long>& blockDeltas, int* rawData, uint64_t &rawIndex, uint8_t blockSize, int minDelta) {
  for (int deltaIndex = 0; deltaIndex < blockSize - 1; deltaIndex++) {
    blockDeltas.push_back(rawData[rawIndex + 1] - rawData[rawIndex]);
    if (blockDeltas[rawIndex] < minDelta) {
      minDelta = blockDeltas[rawIndex];
    }
    rawIndex++;
  }

  return minDelta;
}

//calculates the maxFrameOfReferences which is used to compute the miniBlockBitWidths subtracting the minDelta from the blockDeltas
void calculateMaxFrameOfReference (uint64_t setIndex, uint8_t miniBlockCount, uint8_t miniBlockSize, uint32_t *maxFrameOfReferences, 
                                   uint8_t *miniBlockBitWidths, std::vector<long long>& blockDeltas, int minDelta) {
  for (int miniIndex = 0; miniIndex < miniBlockCount; miniIndex++) {
    maxFrameOfReferences[miniIndex] = 0;
    if (miniIndex == 3) miniBlockSize--; //might have to be different for remainders? check TODO
    for (int valueIndex = 0; valueIndex < miniBlockSize; valueIndex++) {
      blockDeltas[setIndex + valueIndex] -= minDelta;

      if (blockDeltas[setIndex + valueIndex] > maxFrameOfReferences[miniIndex]) {
        maxFrameOfReferences[miniIndex] = blockDeltas[setIndex + valueIndex];
      }
    }
    if (miniIndex == 3) miniBlockSize++;
    miniBlockBitWidths[miniIndex] = (log10(maxFrameOfReferences[miniIndex]) / log10(2)) + 1;
    setIndex += 32;
  }                             
}

void encodeByte(uint8_t byteToEncode, uint8_t &encodedBitsRemaining, uint8_t mask, uint8_t incr, std::vector<long long> &blockDeltas,
                unsigned long long &encodedIndex, std::vector<uint8_t> &encoded) {
  if (encodedBitsRemaining == 0) {
    encoded.push_back(byteToEncode);
    encodedIndex++;
    return;
  }
  encoded[encodedIndex] += (byteToEncode >> (8 - encodedBitsRemaining));
  encoded.push_back((byteToEncode >> encodedBitsRemaining));
}

//In need of being fixed. The assumption being made was that we can add incr to any index in encoded
//and then it will be have its 8th bit set to 1 indicating that there is more bits to read in the uleb. This is a bad assumption.
//it doesnt work because if the first four bits of the index we are working on belong to one uleb, to incr the 8th bit to indicate
//that we want to continue the uleb, we would need to incr the 4th bit of the next 8 bit chuck in encoded, not the current index's
//8th bit. So just adding incr to the current 8 bit block in encoded will usually not generate the correct value. TODO fix. 
void encodeVariableBits(long long intToEncode, uint8_t &encodedBitsRemaining, uint8_t mask, uint8_t incr,
                        std::vector<long long> &blockDeltas, unsigned long long &encodedIndex, std::vector<uint8_t> &encoded) {
  uint8_t bitsConsumed = 0;

  printf("encodedIndex in encodeVariableBits: %lli \nencodedBitsRemaining: %i ", encodedIndex, encodedBitsRemaining);
  while ((intToEncode >> bitsConsumed) > (mask >> encodedBitsRemaining)) { //remaining bits of intToEncode are bigger than the bits left in the current encoded 8 bit block
    if (encodedBitsRemaining > 0) {
      encoded[encodedIndex] += (((intToEncode >> bitsConsumed) & (mask >> (7 - encodedBitsRemaining))) + incr);
      bitsConsumed += encodedBitsRemaining;
      encodedBitsRemaining = 0;
    }
    else {
      encoded.push_back(((intToEncode >> bitsConsumed) & mask) + incr);
      encodedIndex++;
      bitsConsumed += 7;
    }
  }
  if (encodedBitsRemaining > 0) {
    encoded[encodedIndex] += (intToEncode >> bitsConsumed);
    encodedBitsRemaining -= (log10(intToEncode >> bitsConsumed) / log10(2)) + 1; //TODO check (log10(0) / log10(2)) + 1 == 0 is true
  }
  else if ((intToEncode >> bitsConsumed) > 0) {
    encoded.push_back(intToEncode >> bitsConsumed);
    encodedIndex++;
    encodedBitsRemaining = 7 - (log10(intToEncode >> bitsConsumed) / log10(2)) + 1;
  }
  printf("%i\n", encodedBitsRemaining);

}

//This function is for 32 bit ints. We should check the use of 'int' vs 'uint32_t' for signed and unsigned values
//to ensure that there is no truncation. Since the ints that are being passed in are signed, then the use of signed
//ints anywhere else in this fuction should have enough digits to ensure that no precision is lost.
//signed int 32 range should be -2,147,483,648 to 2,147,483,647 while unsiged is 0 to 4,294,967,295.
std::vector<uint8_t> encode32BitData(int* rawData, uint64_t length) {
  std::vector<uint8_t> encoded;
  unsigned long long encodedIndex = 0;

  uint8_t mask = 0x7f; //decimal 127, used with OR to mask out 7 bits
  uint8_t incr = 0x80; //decimal 128, or 1 followed by 7 0's in binary. in uleb128, means take the next 8 bits to continue the int

  uint32_t maxCuttOffs[4] = {0x80, 0x4000, 0x200000, 0x10000000};

  uint8_t blockSize = 128;
  uint8_t iterations = 5;
  uint8_t miniBlockCount = 4;

  uint8_t miniBlockSize = blockSize / miniBlockCount;
  uint8_t byte = 8;

  if (length < 1) {
    return encoded;
  }

  //The header follows the format of 3 ULEB128 ints followed by a zigzag ULEB128 int
  long long header[4] = {blockSize, miniBlockCount, (long long)length, rawData[0]}; //length is a uint64_t so could be trunkated by signed int64_t
  populateHeader(encoded, header, encodedIndex, maxCuttOffs, iterations, mask, incr);

  if (length == 1) { //TODO should be able to remove bc numBlocks will floor to 0 if length < 128 and then block pad.


    return encoded;

  }

  uint64_t rawIndex = 0;
  uint8_t encodedBitsRemaining = 0;
  uint8_t bitsConsumed = 0;
  uint64_t numBlocks = floor(length / blockSize);
  //Start of encoding blocks. The basic flow is to isolate 128 int values that make up a block. Each block
  //contains 4 miniblocks, meaning that every miniblock contains 32 int values.
  for (int blockIndex = 0; blockIndex < numBlocks; blockIndex++) {
    std::vector<long long> blockDeltas; //<long long> is used because deltas can be larger than the original data type
    blockDeltas.reserve(blockSize - 1); //this 127 length vector needs to store blockSize - 1 values

    uint32_t maxFrameOfReferences[miniBlockCount]; //used to compute the bitwidth per miniblock, always postive
    uint8_t miniBlockBitWidths[miniBlockCount]; //bitwidths are stored as a byte and must be postive

    int minDelta = calulateDeltas(blockDeltas, rawData, rawIndex, blockSize, std::numeric_limits<int>::max());
    
    //calculate the minDelta, compute the deltas in blockDeltas, while correctly incrmenting rawIndex
    //Because we are taking the floor of the number of blocks, if we get to this point there is an even multiple
    //of 128 int values to compute on meaning we do not have to check if we have values left. So we do not have
    //to check length.

    //calculate maxFrameOfReference per miniBlock and compute miniBlockBitWidth per miniBlock using MFOR
    uint64_t setIndex = rawIndex - blockSize + 1; //this value may need a + 1 attached as we have blockSize - 1 deltas,
    //so rawIndex is only incr'ed blockSize - 1 times TODO: Check ^^^

    //calculate maxFrameOfReference and miniBlockBitWidths for each miniBlock and subtract the minDelta from each value in blockDelta
    calculateMaxFrameOfReference(setIndex, miniBlockCount, miniBlockSize, maxFrameOfReferences, miniBlockBitWidths, blockDeltas, minDelta);

    //We now have all that we need to encode the block. minDelta, the four miniBlockBitWidths, 
    //and the miniblocks themselves. 
    //block flow: <mindelta> encoded as a zig zag ULEB128 int, <bitwidth> stored as a byte, and <miniblocks>
    //which is a list of bit packed ints of bitwidth length. All of these need to be stored in unsigend 8 bit
    //integers of range 0 to 255.
    minDelta = computeZigZag(minDelta);
    encodeVariableBits(minDelta, encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);
    //minDelta encoded.

    //the use of the encodeByte function will need to be changed to encodeVariableBits() as we do not have 
    //8 bit alignment in every case
    for(int y = 0; y < miniBlockCount; y++) {
      encodeByte(miniBlockBitWidths[y], encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);
    }
    //miniBlockBitWidths encoded.

    for (int miniIndex = 0; miniIndex < miniBlockCount; miniIndex++) {
      int miniOffset = miniBlockSize * miniIndex;

      for (int valueIndex = 0; valueIndex < miniBlockSize; valueIndex++) {
        encodeVariableBits(blockDeltas[valueIndex + miniOffset], encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);
      }

    }
    //deltas encoded
  }

  //floor(legth / blockSize) blocks are now encoded and we finish with the remainder.
  //same as block before but use modulo to find how many real values we have. after bit padd with 0's
  //(just the last miniblock I think)
  //new blockSize for remaining lenght:

  blockSize = length % blockSize; //blockSize now holds the remaining length

  std::vector<long long> blockDeltas; 
  blockDeltas.reserve(blockSize - 1);

  uint32_t maxFrameOfReferences[miniBlockCount]; 
  uint8_t miniBlockBitWidths[miniBlockCount]; 

  int minDelta = calulateDeltas(blockDeltas, rawData, rawIndex, blockSize, std::numeric_limits<int>::max());

  //blockDeltas populated with remaining length of items

  uint64_t setIndex = rawIndex - blockSize + 1; 

  //need to change miniBlockCount(per block) and miniBlockSize to reflect the remaining length.
  miniBlockCount = ceil(blockSize / (double)miniBlockSize);
  miniBlockSize = blockSize % miniBlockSize;
  if (miniBlockSize == 0) miniBlockSize = 32; //only 0 if blockSize is a multiple of 32, never if it's just 0, would not branch here (i hope)
  miniBlockSize--;
  uint8_t defaultMiniBlockSize = 32;
  uint8_t defaultMiniBlockCount = 4;

  calculateMaxFrameOfReference(setIndex, miniBlockCount, miniBlockSize, maxFrameOfReferences, miniBlockBitWidths, blockDeltas, minDelta);

  //processing that remaining maxFrameOfReferences are padded with 0 when blockSize(remaining length) runs out
  for (int remainder = miniBlockCount; remainder < defaultMiniBlockCount; remainder++) {
    maxFrameOfReferences[remainder] = 0;
    miniBlockBitWidths[remainder] = 0;
  }

  minDelta = computeZigZag(minDelta);

  encodeVariableBits(minDelta, encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);

  printf("miniblock bit widths\n");
  //This portion of encoding the miniblockbitwidths will need to be changed from the encodeByte funciton to the
  //encodeVariableBits function because we do not have a guarantee of 8 bit alignment.
  for(int y = 0; y < defaultMiniBlockCount; y++) {
    encodeByte(miniBlockBitWidths[y], encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);
    printf("%i ", miniBlockBitWidths[y]);
  }
  printf("\n");
  //miniBlockBitWidths encoded.

  for (int remainingValues = 0; remainingValues < blockSize; remainingValues++) {
    encodeVariableBits(blockDeltas[remainingValues], encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);
  }

  //add in the last remianing values and then pad to miniBlock 32 value increment
  for (int pad = 32 - miniBlockSize; pad < 32; pad++) {
    encodeVariableBits(0, encodedBitsRemaining, mask, incr, blockDeltas, encodedIndex, encoded);
  }

  printf("encoded:\n");

  for (int i = 0; i < encodedIndex; i++) {
    printf("%i ", encoded[i]);
  } printf("\n");

  return encoded;
}
