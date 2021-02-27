#include "PostTagCribFile.hpp"

#include <boost/format.hpp>

PostTagCribFile PostTagCribFileReader::read_file() {
  uint8_t file_magic = read_u8();
  PostTagFileMagic format_magic = CribFileMagic;

  if (file_magic != format_magic) {
    throw std::runtime_error((boost::format("File magic number 0x%X did not match expected 0x%X") %
                              static_cast<unsigned int>(file_magic) % static_cast<unsigned int>(format_magic))
                                 .str());
  }

  uint8_t version = read_u8();
  switch (version) {
    case V1:
      return read_file_V1();

    default:
      throw std::runtime_error((boost::format("Unknown file version %u") % static_cast<unsigned int>(version)).str());
  }
}

PostTagCribFile PostTagCribFileReader::read_file_V1() {
  PostTagCribFile file;
  file.version = V1;
  file.sequence_count = read_u64();
  file.sequences = read_sequences(file.sequence_count);

  return file;
}

std::vector<std::vector<bool>> PostTagCribFileReader::read_sequences(uint64_t sequence_count) {
  std::vector<std::vector<bool>> sequences(sequence_count);
  for (size_t i = 0; i < sequence_count; i++) {
    sequences[i] = read_prefixed_bits();
  }

  return sequences;
}
