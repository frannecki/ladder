#ifndef LADDER_UTILS_H
#define LADDER_UTILS_H

#include <stdio.h>

#include <string>
#include <vector>

namespace ladder {

const int kEnableOption = 1;

#define EXIT fprintf(stderr, "[%s:%u %s] ", __FILE__, __LINE__, __FUNCTION__); exit_fatal

void exit_fatal(const char* msg);

std::vector<int> FindSubstr(const std::string& str,
                            const std::string& pat);

bool CheckIfFileExists(const std::string& path);

int GetFileSize(const std::string& path);

} // namespace ladder

#endif
