#ifndef LADDER_CONNECTION_H
#define LADDER_CONNECTION_H

#include <mutex>
#include <string>

#include <Aliases.h>

namespace ladder
{

class Connection : public std::enable_shared_from_this<Connection> {

public:
  Connection(const EventLoopPtr& loop, int fd);
  ~Connection();
  void Init();
  void SetChannelCallbacks();
  void Send(const std::string& buf);
  void OnReadCallback();
  void OnWriteCallback();
  void OnCloseCallback();
  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  void SetCloseCallback(const ConnectCloseCallback& callback);
  ChannelPtr channel() const;

private:
  std::mutex mutex_;
  // cannot use raw pointer here, because Channel would call shared_from_this()
  ChannelPtr channel_;
  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  ConnectCloseCallback close_callback_;
  Buffer* read_buffer_;
  Buffer* write_buffer_;
  bool shut_down_;
};

} // namespace ladder


#endif
