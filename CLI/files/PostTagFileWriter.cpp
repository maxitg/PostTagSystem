#include "PostTagFileWriter.hpp"

void PostTagFileWriter::write_u8(uint8_t n) {
  put(n);

  if (bad()) {
    throw std::logic_error("Failed to write to file");
  }
}

void PostTagFileWriter::write_u16(uint16_t n) {
  write_u8(0xFF & (n >> 0));
  write_u8(0xFF & (n >> 8));
}

void PostTagFileWriter::write_u32(uint32_t n) {
  write_u16(0xFFFF & (n >> 0));
  write_u16(0xFFFF & (n >> 16));
}

void PostTagFileWriter::write_u64(uint64_t n) {
  write_u32(0xFFFFFFFF & (n >> 0));
  write_u32(0xFFFFFFFF & (n >> 32));
}

void PostTagFileWriter::write_bits(std::vector<bool> bits) {
  uint64_t full_bytes = bits.size() / 8;
  uint8_t remainder_bits = bits.size() % 8;

  // process all the full bytes
  for (size_t byte_index = 0; byte_index < full_bytes; byte_index++) {
    uint8_t byte = 0;
    for (size_t bit = 0; bit < 8; bit++) {
      byte |= bits[(byte_index * 8) + bit] << (7 - bit);
    }
    write_u8(byte);
  }

  // process the single partial byte at the end, if applicable
  if (remainder_bits > 0) {
    uint8_t last_byte = 0;
    for (size_t bit = 0; bit < remainder_bits; bit++) {
      last_byte |= bits[(full_bytes * 8) + bit] << (7 - bit);
    }
    write_u8(last_byte);
  }
}

void PostTagFileWriter::write_prefixed_bits(std::vector<bool> bits) {
  write_u64(bits.size());
  write_bits(bits);
}
