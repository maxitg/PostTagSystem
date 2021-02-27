#ifndef CLI_FILES_POSTTAGCRIBFILE_HPP_
#define CLI_FILES_POSTTAGCRIBFILE_HPP_

#include <fstream>
#include <vector>

#include "PostTagFileReader.hpp"
#include "PostTagState.hpp"

struct PostTagCribFile {
  PostTagFileVersion version;
  uint64_t checkpoint_count;

  std::vector<PostTagSystem::PostTagState> checkpoints;
};

class PostTagCribFileReader : public PostTagFileReader {
 public:
  using PostTagFileReader::PostTagFileReader;

  PostTagCribFile read_file();

 private:
  PostTagCribFile read_file_V1();

  std::vector<PostTagSystem::PostTagState> read_checkpoints(uint64_t checkpoint_count);
};

#endif  // CLI_FILES_POSTTAGCRIBFILE_HPP_
