/*******************************************************************************
* Copyright (C) 2019 - 2023, winsoft666, <winsoft666@outlook.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
* Expect bugs
*
* Please use and enjoy. Please let me know of any bugs/improvements
* that you have found/implemented and I will fix/incorporate them into this
* file.
*******************************************************************************/

#include "file_util.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace teemo {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

long GetFileSize(FILE *f) {
  if (!f)
    return 0;
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  return fsize;
}

std::string GetDirectory(const std::string &path) {
  std::string::size_type pos = path.find_last_of(PATH_SEPARATOR);
  return path.substr(0, pos);
}

std::string GetFileName(const std::string &path) {
  std::string::size_type pos = path.find_last_of(PATH_SEPARATOR);
  if (pos == std::string::npos)
    pos = 0;
  else
    pos++;
  return path.substr(pos);
}

std::string AppendFileName(const std::string &dir, const std::string &filename) {
  std::string result = dir;
  if (result.length() > 0) {
    if (result[result.length() - 1] != PATH_SEPARATOR)
      result += PATH_SEPARATOR;
  }

  result += filename;
  return result;
}

bool FileIsExist(const std::string &filepath) {
#if (defined WIN32 || defined _WIN32)
  return (access(filepath.c_str(), 0) == 0);
#else
  return (access(filepath.c_str(), F_OK) == 0);
#endif
}

bool FileIsRW(const std::string &filepath) {
#if (defined WIN32 || defined _WIN32)
  return (access(filepath.c_str(), 6) == 0);
#else
  return (access(filepath.c_str(), R_OK | W_OK) == 0);
#endif
}

} // namespace teemo
