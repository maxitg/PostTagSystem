#ifndef CLI_FILES_POSTTAGCRIBFILE_HPP_
#define CLI_FILES_POSTTAGCRIBFILE_HPP_

#include <fstream>
#include <vector>

#include "PostTagFileReader.hpp"

struct PostTagCribFile {
  PostTagFileVersion version;
  uint64_t sequence_count;

  std::vector<std::vector<bool>> sequences;
};

class PostTagCribFileReader : public PostTagFileReader {
 public:
  using PostTagFileReader::PostTagFileReader;

  PostTagCribFile read_file();

 private:
  PostTagCribFile read_file_V1();

  std::vector<std::vector<bool>> read_sequences(uint64_t sequence_count);
};

#endif  // CLI_FILES_POSTTAGCRIBFILE_HPP_
