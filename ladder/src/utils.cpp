#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <MemoryPool.h>
#include <utils.h>
namespace ladder {

void exit_fatal(const char* msg) {
  perror(msg);
  exit(-1);
}

std::vector<int> FindSubstr(const std::string& str,
                                   const std::string& pat)
{
    int len1 = str.size();
    int len2 = pat.size();
    std::vector<int> positions;
    
    MemoryWrapper<int> wrapper(1 + len2);
    int* next = wrapper.get();

    int t = next[0] = -1, i = 0, j;
    while (i < len2) {
      if (t < 0 || pat[t] == pat[i]) {
        ++i;
        int tmp = ++t;
        while (tmp >= 0 && pat[tmp] == pat[i]) {
          tmp = next[tmp];
        }
        next[i] = tmp;
      }
      else t = next[t];
    }
    i = j = 0;
    while (i + len2 <= len1 && j <= len2) {
      if ( j < len2 && (j < 0 || str[i+j] == pat[j])) {
        ++j;
      }
      else {
        if (j == len2) {
          positions.push_back(i);
        }
        i += j - next[j];
        j = next[j];
      }
    }
    next = nullptr;

    return positions;
}

bool CheckIfFileExists(const std::string& path) {
  return access(path.c_str(), F_OK | R_OK) == 0;
}

int GetFileSize(const std::string& path) {
  FILE* fp = fopen(path.c_str(), "r");
  if(fp == NULL) {
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  fclose(fp);
  return size;
}


} // namespace ladder
