#ifndef LADDER_CONNECTION_H
#define LADDER_CONNECTION_H

#include <string>

#include "compat.h"

#ifdef LADDER_OS_WINDOWS
#include <winSock2.h>
#endif

#include "Base.h"
#include "Socket.h"

namespace ladder {

class FileBuffer;

class LADDER_API Connection : public std::enable_shared_from_this<Connection> {
 public:
#ifdef LADDER_OS_WINDOWS
  Connection(int fd, bool send_file = false);
  virtual void Init(HANDLE iocp_port, char* buffer, int io_size);
  void OnReadCallback(int io_size);
  void OnWriteCallback(int io_size);
#else
  Connection(const EventLoopPtr& loop, int fd, bool send_file = false);
  virtual void Init();
  void OnReadCallback();
  void OnWriteCallback();
#endif
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;
  virtual ~Connection();
  void SetChannelCallbacks();
  virtual void Send(const std::string& buf);
  void ShutDownWrite();
  void Close();
  void SendFile(std::string&& header, const std::string& filename = "");
  void OnCloseCallback();
  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  void SetCloseCallback(const ConnectCloseCallback& callback);
  ChannelPtr channel() const;
  void LockCallback(bool lock = true);

 protected:
#ifdef LADDER_OS_WINDOWS
  virtual int ReadBuffer(int io_size);
  virtual int WriteBuffer(int io_size);
  void PostRead();
  void PostWrite();
  
  LPWSABUF read_wsa_buf_;
  LPWSABUF write_wsa_buf_;
  SocketIocpStatus* read_status_;
  SocketIocpStatus* write_status_;
  bool write_pending_;  // iocp write operation pending
#else
  virtual int ReadBuffer();
  virtual int WriteBuffer();
#endif
  void EnableWrite(int ret);

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

  std::mutex mutex_callback_;
};

}  // namespace ladder

#endif
