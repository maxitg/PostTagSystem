#ifndef CLI_FILES_POSTTAGRESULTFILE_HPP_
#define CLI_FILES_POSTTAGRESULTFILE_HPP_

#include <fstream>
#include <vector>

#include "PostTagFile.hpp"
#include "PostTagFileWriter.hpp"
#include "PostTagSearcher.hpp"

struct PostTagResultFile {
  PostTagFileVersion version;

  uint64_t result_count;
  const std::vector<PostTagSystem::PostTagSearcher::EvaluationResult>& results;

  PostTagResultFile(PostTagFileVersion v, const std::vector<PostTagSystem::PostTagSearcher::EvaluationResult>& r)
      : version(v), results(r) {
    result_count = results.size();
  }
};

class PostTagResultFileWriter : public PostTagFileWriter {
 public:
  using PostTagFileWriter::PostTagFileWriter;

  void write_file(const PostTagResultFile& file);

 private:
  void write_file_V1(const PostTagResultFile& file);

  void write_result(const PostTagSystem::PostTagSearcher::EvaluationResult& result, bool write_final_state);
};

#endif  // CLI_FILES_POSTTAGRESULTFILE_HPP_
