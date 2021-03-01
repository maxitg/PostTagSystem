#include "PostTagFileReader.hpp"

uint8_t PostTagFileReader::read_u8() {
  uint8_t byte = static_cast<uint8_t>(get());

  if (eof()) {
    throw std::logic_error("Unexpected EOF");
  } else {
    return byte;
  }
}

uint16_t PostTagFileReader::read_u16() {
  uint16_t n = 0;
  n |= (read_u8() << 0);
  n |= (read_u8() << 8);

  return n;
}

uint32_t PostTagFileReader::read_u32() {
  uint32_t n = 0;
  n |= (read_u16() << 0);
  n |= (read_u16() << 16);

  return n;
}

uint64_t PostTagFileReader::read_u64() {
  uint64_t n = 0;
  n |= (read_u32() << 0);
  n |= (static_cast<uint64_t>(read_u32()) << 32);

  return n;
}

std::vector<bool> PostTagFileReader::read_bits(uint64_t bit_count) {
  std::vector<bool> bits(bit_count, false);

  uint64_t full_bytes = bit_count / 8;
  uint8_t remainder_bits = bit_count % 8;

  // process all the full bytes
  for (size_t byte_index = 0; byte_index < full_bytes; byte_index++) {
    uint8_t byte = read_u8();
    for (size_t bit = 0; bit < 8; bit++) {
      bits[(byte_index * 8) + bit] = (byte >> (7 - bit)) & 1;
    }
  }

  // process the single partial byte at the end, if applicable
  if (remainder_bits > 0) {
    uint8_t last_byte = read_u8();
    for (size_t bit = 0; bit < remainder_bits; bit++) {
      bits[(full_bytes * 8) + bit] = (last_byte >> (7 - bit)) & 1;
    }
  }

  return bits;
}

std::vector<bool> PostTagFileReader::read_prefixed_bits() { return read_bits(read_u64()); }

std::vector<bool> PostTagFileReader::read_bits_u64(uint8_t bit_count) {
  std::vector<bool> bits(bit_count, false);
  uint64_t bits_dec = read_u64();

  for (int8_t bit = (bit_count - 1); bit >= 0; bit--) {
    bits[bit] = bits_dec & 1;
    bits_dec >>= 1;
  }

  return bits;
}
