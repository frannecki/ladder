#ifndef LADDER_CALLBACKS_H
#define LADDER_CALLBACKS_H

#include <functional>

namespace ladder {

class Connection;
class Buffer;

using ConnectionPtr = std::shared_ptr<Connection>;

using ReadEvtCallback = std::function<void(const ConnectionPtr&, Buffer*)>;
using WriteEvtCallback = std::function<void(Buffer*)>;

} // namespace ladder

#endif
