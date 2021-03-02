#ifndef CLI_FILES_POSTTAGFILEWRITER_HPP_
#define CLI_FILES_POSTTAGFILEWRITER_HPP_

#include <fstream>
#include <vector>

class PostTagFileWriter : public std::ofstream {
 public:
  using std::ofstream::ofstream;

  void write_u8(uint8_t n);
  void write_u16(uint16_t n);  // all multi-byte values are stored in little-endian format
  void write_u32(uint32_t n);
  void write_u64(uint64_t n);

  void write_bits(const std::vector<bool>& bits);

  // bit sequence prefixed by a uint_64 indicating the # of bits
  void write_prefixed_bits(const std::vector<bool>& bits);

  // sequence of <= 64 bits written as a 64-bit decimal integer
  void write_bits_u64(const std::vector<bool>& bits);
};

#endif  // CLI_FILES_POSTTAGFILEWRITER_HPP_
