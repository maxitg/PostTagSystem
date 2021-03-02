#ifndef CLI_FILES_POSTTAGFILE_HPP_
#define CLI_FILES_POSTTAGFILE_HPP_

enum PostTagFileVersion : uint8_t { Version1 = 1 };

enum PostTagFileMagic : uint8_t { CribFileMagic = 'C', InitFileMagic = 'I', ResultFileMagic = 'R' };

#endif  // CLI_FILES_POSTTAGFILE_HPP_
