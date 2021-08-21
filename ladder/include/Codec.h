#ifndef LADDER_CODEC_H
#define LADDER_CODEC_H

#include <map>
#include <functional>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>


namespace ladder {

using Callback = std::function<void()>;

class Codec {
public:
  Codec();

private:
  std::map<google::protobuf::Descriptor*, Callback> callbacks_;
};

} // namespace ladder

#endif
