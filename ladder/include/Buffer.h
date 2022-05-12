#ifndef LADDER_BUFFER_H
#define LADDER_BUFFER_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mutex>
#include <string>
#include <vector>

#include "Base.h"

namespace ladder {

class LADDER_API Buffer : public IBuffer {
 public:
  Buffer();
  std::string Read(size_t n);
  std::string ReadAll();
  void Write(const std::string& content) override;
  void Write(const char* src, size_t len) override;
  uint32_t ReadUInt32();
  uint32_t PeekUInt32();
  void WriteUInt32(uint32_t number);
  uint32_t ReadableBytes() const;
#ifndef LADDER_OS_WINDOWS
  int ReadBufferFromFd(int fd);
  int WriteBufferToFd(int fd) override;
#endif
  bool Empty() const override;

  uint32_t Peek(size_t n, std::string& result);
  uint32_t Peek(char* dst, size_t len) override;
  void HaveRead(size_t n) override;

 private:
  uint32_t read_index_, write_index_;
  std::vector<char> buffer_;

  std::mutex mutex_;
};

}  // namespace ladder

#endif
