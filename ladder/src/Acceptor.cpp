#include <functional>

#include <Acceptor.h>
#include <Channel.h>
#include <utils.h>

namespace ladder {

Acceptor::Acceptor(const ChannelPtr& channel) :
  channel_(channel)
{
  channel->SetReadCallback(std::bind(&Acceptor::HandleAccept, this));
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& callback) {
  new_connection_callback_ = callback;
}

void Acceptor::HandleAccept() {
  sockaddr_t addr;
  int fd = socket::accept(channel_->fd(), &addr);
  new_connection_callback_(fd);
}

} // namespace ladder
