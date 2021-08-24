#ifndef LADDER_BUFFER_H
#define LADDER_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vector>
#include <string>
#include <mutex>

namespace ladder {

class Buffer {
public:
  Buffer();
  std::string Read(size_t n);
  std::string ReadAll();
  void Write(const std::string& content);
  uint32_t ReadUInt32();
  void WriteUInt32(uint32_t number);
  uint32_t ReadableBytes() const;
  void Prepend(uint32_t number);

  int ReadBufferFromFd(int fd);
  void WriteBufferToFd(int fd);

private:
  uint32_t read_index_, write_index_;
  std::vector<char> buffer_;
  void Prepend(const char* data, size_t len);
  uint32_t Peek(size_t n, std::string& result);
  void HaveRead(size_t n);
  
  std::mutex mutex_;
};

} // namespace ladder

#endif
