#include "PostTagResultFile.hpp"

#include <boost/format.hpp>

using PostTagSystem::PostTagHistory;

void PostTagResultFileWriter::write_file(const PostTagResultFile& file) {
  // write magic number and version
  write_u8(PostTagFileMagic::ResultFileMagic);
  write_u8(file.version);

  switch (file.version) {
    case Version1:
      return write_file_V1(file);

    default:
      throw std::runtime_error(
          (boost::format("Unsupported file version %u") % static_cast<unsigned int>(file.version)).str());
  }
}

void PostTagResultFileWriter::write_file_V1(const PostTagResultFile& file) {
  write_u64(file.result_count);

  for (size_t i = 0; i < file.result_count; i++) {
    // TODO(jessef): set write_final_state based on
    // disposition and/or tape size and/or other stuff
    write_result(file.results[i], true);
  }
}

void PostTagResultFileWriter::write_result(const PostTagHistory::EvaluationResult& result, bool write_final_state) {
  // TODO(jessef): write initial state

  uint8_t flags = 0;
  // TODO(jessef): set upper 5 bits of flag to disposition
  flags |= (write_final_state & 1) << 0;  // last bit
  write_u8(flags);

  write_u64(result.eventCount);
  write_u64(result.maxTapeLength);

  write_u64(result.finalState.tape.size());  // TODO(jessef): change to result.finalTapeLength

  if (write_final_state) {
    write_u8(result.finalState.headState);

    // no need to write a prefix since we already wrote the final length
    write_bits(result.finalState.tape);
  }
}
