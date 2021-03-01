#include "PostTagResultFile.hpp"

#include "TagState.hpp"
#include "boost/format.hpp"

using PostTagSystem::PostTagSearcher;

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

  for (const PostTagSearcher::EvaluationResult& result : file.results) {
    // include the final state if it's not too big and the reason isn't one where it's irrelevant
    bool write_final_state = result.finalTapeLength <= file.biggest_tape_to_write;
    switch (result.conclusionReason) {
      case PostTagSearcher::ConclusionReason::InvalidInput:
      case PostTagSearcher::ConclusionReason::NotEvaluated:
        write_final_state = false;
        break;
      
      default:
        break;
    }

    write_result(result, write_final_state);
  }
}

void PostTagResultFileWriter::write_result(const PostTagSearcher::EvaluationResult& result, bool write_final_state) {
  // write initial state
  uint8_t initial_state_header = 0;
  initial_state_header |= (result.initialState.headState & 0b11) << 6;       // bits 0-1: initial head state
  initial_state_header |= (result.initialState.tape.size() - 1) & 0b111111;  // bits 2-7: initial tape length minus 1
  write_u8(initial_state_header);

  write_bits(result.initialState.tape);

  uint8_t result_header = 0;
  result_header |= (static_cast<uint8_t>(result.conclusionReason) & 0b11111) << 3;  // bits 0-4: conclusion reason
  result_header |= (result.finalState.headState & 0b11) << 1;                       // bits 5-6: final head state
  result_header |= (write_final_state & 1) << 0;  // bit 7: whether or not final state follows
  write_u8(result_header);

  write_u64(result.eventCount);
  write_u64(result.maxTapeLength);
  write_u64(result.finalTapeLength);

  if (write_final_state) {
    // no need to write a prefix since we already wrote the final length
    write_bits(result.finalState.tape);
  }
}
