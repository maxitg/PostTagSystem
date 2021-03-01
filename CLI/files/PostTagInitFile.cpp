#include "PostTagInitFile.hpp"

#include <boost/format.hpp>

using PostTagSystem::TagState;

PostTagInitFile PostTagInitFileReader::read_file() {
  uint8_t file_magic = read_u8();
  PostTagFileMagic format_magic = InitFileMagic;

  if (file_magic != format_magic) {
    throw std::runtime_error((boost::format("File magic number 0x%X did not match expected 0x%X") %
                              static_cast<unsigned int>(file_magic) % static_cast<unsigned int>(format_magic))
                                 .str());
  }

  uint8_t version = read_u8();
  switch (version) {
    case Version1:
      return read_file_V1();

    default:
      throw std::runtime_error(
          (boost::format("Unsupported file version %u") % static_cast<unsigned int>(version)).str());
  }
}

PostTagInitFile PostTagInitFileReader::read_file_V1() {
  PostTagInitFile file;
  file.version = Version1;
  file.state_count = read_u64();
  file.states = read_states(file.state_count);

  return file;
}

std::vector<TagState> PostTagInitFileReader::read_states(uint64_t state_count) {
  std::vector<TagState> states(state_count);
  for (size_t i = 0; i < state_count; i++) {
    uint8_t state_header = read_u8();
    states[i].headState = (state_header & 0b11000000) >> 6;

    uint8_t tape_length = (state_header & 0b00111111) + 1;
    states[i].tape.resize(tape_length);

    uint64_t tape_dec = read_u64();
    for (int8_t bit = (tape_length - 1); bit >= 0; bit--) {
      states[i].tape[bit] = tape_dec & 1;
      tape_dec >>= 1;
    }
  }

  return states;
}
