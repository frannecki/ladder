#ifndef LADDER_CODEC_H
#define LADDER_CODEC_H

#include <map>
#include <functional>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <Aliases.h>

namespace ladder {

class Callback {
public:
  virtual ~Callback() = default;
  virtual void OnMessage(const ConnectionPtr& conn,
                         google::protobuf::Message* message) = 0;
};

template <typename MessageType>
class CallbackT : public Callback {

public:
  using ProtobufMessageCallbackT = std::function<void(const ConnectionPtr&,
                                                    MessageType*)>;

  CallbackT(const ProtobufMessageCallbackT& callback) : callback_(callback) {};

  void OnMessage(const ConnectionPtr& conn,
                 google::protobuf::Message* message) override
  {
    if(message && callback_) {
      callback_(conn, static_cast<MessageType*>(message));
    }
  }

  ProtobufMessageCallbackT callback_;

};

class ProtobufCodec {

using CallbackPtr = std::shared_ptr<Callback>;

public:
  ProtobufCodec();

  void Send(const ConnectionPtr& conn, google::protobuf::Message* message) const;

  void OnMessage(const ConnectionPtr& conn, Buffer* buffer);

  void RegisterDefaultMessageCallback(
    const std::function<void(const ConnectionPtr&, google::protobuf::Message*)>& callback);

  template <typename MessageType>
  void RegisterMessageCallback(const typename CallbackT<MessageType>::ProtobufMessageCallbackT& callback)
  {
    callbacks_[MessageType::descriptor()] = std::make_shared<CallbackT<MessageType>>(callback);
  }

  static uint32_t kMinMessageLength;

private:
  void ComposeMessage(google::protobuf::Message* message, std::string& buf) const;
  int ParseMessage(google::protobuf::Message*& message, Buffer* buffer);
  
  std::map<const google::protobuf::Descriptor*, CallbackPtr> callbacks_;
  std::function<void(const ConnectionPtr&, google::protobuf::Message*)> default_callback_;
};

} // namespace ladder

#endif
