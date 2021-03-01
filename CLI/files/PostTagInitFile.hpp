#ifndef CLI_FILES_POSTTAGINITFILE_HPP_
#define CLI_FILES_POSTTAGINITFILE_HPP_

#include <fstream>
#include <vector>

#include "PostTagFile.hpp"
#include "PostTagFileReader.hpp"
#include "TagState.hpp"

struct PostTagInitFile {
  PostTagFileVersion version;

  uint64_t state_count;
  std::vector<PostTagSystem::TagState> states;  // TODO(jessef): use SmallState
};

class PostTagInitFileReader : public PostTagFileReader {
 public:
  using PostTagFileReader::PostTagFileReader;

  PostTagInitFile read_file();

 private:
  PostTagInitFile read_file_V1();

  std::vector<PostTagSystem::TagState> read_states(uint64_t state_count);
};

#endif  // CLI_FILES_POSTTAGINITFILE_HPP_
