#ifndef CLI_FILES_POSTTAGFILEREADER_HPP_
#define CLI_FILES_POSTTAGFILEREADER_HPP_

#include <fstream>
#include <vector>

class PostTagFileReader : public std::ifstream {
 public:
  using std::ifstream::ifstream;

  uint8_t read_u8();
  uint16_t read_u16();  // all multi-byte values are stored in little-endian format
  uint32_t read_u32();
  uint64_t read_u64();

  std::vector<bool> read_bits(uint64_t bit_count);

  // bit sequence prefixed by a uint_64 indicating the # of bits
  std::vector<bool> read_prefixed_bits();
};

#endif  // CLI_FILES_POSTTAGFILEREADER_HPP_
