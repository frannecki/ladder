#ifndef LADDER_BUFFER_H
#define LADDER_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vector>
#include <string>
#include <mutex>

#include <Base.h>

namespace ladder {

class Buffer : public IBuffer {
public:
  Buffer();
  std::string Read(size_t n);
  std::string ReadAll();
  void Write(const std::string& content) override;
  uint32_t ReadUInt32();
  uint32_t PeekUInt32();
  void WriteUInt32(uint32_t number);
  uint32_t ReadableBytes() const;

  int ReadBufferFromFd(int fd);
  int WriteBufferToFd(int fd) override;
  bool Empty() const override;

  uint32_t Peek(size_t n, std::string& result);
  void HaveRead(size_t n);

private:
  uint32_t read_index_, write_index_;
  std::vector<char> buffer_;
  
  std::mutex mutex_;
};

} // namespace ladder

#endif
