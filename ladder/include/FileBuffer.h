#ifndef LADDER_FILE_BUFFER_H
#define LADDER_FILE_BUFFER_H

#include <queue>
#include <string>

#include <Base.h>

namespace ladder {

struct FileInfo {
  std::string header_;
  std::string filename_;
  FileInfo(std::string&& header, const std::string& filename) : 
    header_(header), filename_(filename)
  {

  }
};

class Buffer;

class FileBuffer : public IBuffer {

public:
  FileBuffer();
  ~FileBuffer();
  void AddFile(std::string&& header,
               const std::string& filename);
  void Write(const std::string& content) override;
  int WriteBufferToFd(int fd) override;
  bool Empty() const override;

private:
  std::queue<FileInfo> pending_files_;
  Buffer* buffer_;
  int bytes_sent_;
  int bytes_pending_;
  int fd_;

};

} // namespace ladder

#endif
