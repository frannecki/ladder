#ifndef LADDER_CONNECTION_H
#define LADDER_CONNECTION_H

#include <mutex>
#include <string>

#include <Base.h>

namespace ladder {

class FileBuffer;

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  Connection(const EventLoopPtr& loop, int fd, bool send_file = false);
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;
  virtual ~Connection();
  virtual void Init();
  void SetChannelCallbacks();
  virtual void Send(const std::string& buf);
  void ShutDownWrite();
  void Close();
  void SendFile(std::string&& header, const std::string& filename = "");
  void OnReadCallback();
  void OnWriteCallback();
  void OnCloseCallback();
  void set_read_callback(const ReadEvtCallback& callback);
  void set_write_callback(const WriteEvtCallback& callback);
  void set_close_callback(const ConnectCloseCallback& callback);
  ChannelPtr channel() const;

 protected:
  virtual int ReadBuffer();
  virtual int WriteBuffer();
  void EnableWrite(int ret);

  std::mutex mutex_;
  // cannot use raw pointer here, because Channel would call shared_from_this()
  ChannelPtr channel_;
  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  ConnectCloseCallback close_callback_;
  Buffer* read_buffer_;

  IBuffer* write_buffer_;
  Buffer* write_plain_buffer_;
  FileBuffer* write_file_buffer_;

  // buffer for writing file, taking advantage of the `sendfile` linux/bsd
  // system call
  bool shut_down_;
  bool send_file_;
};

}  // namespace ladder

#endif
