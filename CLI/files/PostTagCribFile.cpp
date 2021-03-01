#include "PostTagCribFile.hpp"

#include <boost/format.hpp>

using PostTagSystem::TagState;

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
    case Version1:
      return read_file_V1();

    default:
      throw std::runtime_error(
          (boost::format("Unsupported file version %u") % static_cast<unsigned int>(version)).str());
  }
}

PostTagCribFile PostTagCribFileReader::read_file_V1() {
  PostTagCribFile file;
  file.version = Version1;
  file.checkpoint_count = read_u64();
  file.checkpoints = read_checkpoints(file.checkpoint_count);

  return file;
}

std::vector<TagState> PostTagCribFileReader::read_checkpoints(uint64_t checkpoint_count) {
  std::vector<TagState> checkpoints(checkpoint_count);
  for (size_t i = 0; i < checkpoint_count; i++) {
    checkpoints[i].headState = read_u8();
    checkpoints[i].tape = read_prefixed_bits();
  }

  return checkpoints;
}
