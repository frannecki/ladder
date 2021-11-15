#ifndef LADDER_CONNECTION_H
#define LADDER_CONNECTION_H

#include <mutex>
#include <string>

#ifdef _MSC_VER
#include <winSock2.h>
#endif

#include <Base.h>
#include <Socket.h>

namespace ladder {

class FileBuffer;

class LADDER_API Connection : public std::enable_shared_from_this<Connection> {
 public:
#ifdef _MSC_VER
  Connection(int fd, bool send_file = false);
#else
  Connection(const EventLoopPtr& loop, int fd, bool send_file = false);
#endif
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;
  virtual ~Connection();
#ifdef _MSC_VER
  virtual void Init(HANDLE iocp_port, char* buffer, int io_size);
#else
  virtual void Init();
#endif
  void SetChannelCallbacks();
  virtual void Send(const std::string& buf);
  void ShutDownWrite();
  void Close();
  void SendFile(std::string&& header, const std::string& filename = "");
#ifdef _MSC_VER
  void OnReadCallback(int io_size);
  void OnWriteCallback(int io_size);
#else
  void OnReadCallback();
  void OnWriteCallback();
#endif
  void OnCloseCallback();
  void set_read_callback(const ReadEvtCallback& callback);
  void set_write_callback(const WriteEvtCallback& callback);
  void set_close_callback(const ConnectCloseCallback& callback);
  ChannelPtr channel() const;

 protected:
#ifdef _MSC_VER
  virtual int ReadBuffer(int io_size);
  virtual int WriteBuffer(int io_size);
  void PostRead();
  void PostWrite();
#else
  virtual int ReadBuffer();
  virtual int WriteBuffer();
#endif
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

#ifdef _MSC_VER
  LPWSABUF read_wsa_buf_;
  LPWSABUF write_wsa_buf_;
  SocketIocpStatus* read_status_;
  SocketIocpStatus* write_status_;
  bool first_sent_; // first bytes sent
#endif

  // buffer for writing file, taking advantage of the `sendfile` linux/bsd
  // system call
  bool shut_down_;
  bool immediate_shut_down_;
  bool send_file_;
};

}  // namespace ladder

#endif
