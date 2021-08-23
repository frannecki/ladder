#ifndef LADDER_CALLBACKS_H
#define LADDER_CALLBACKS_H

#include <functional>

namespace ladder {

class Connection;
class Buffer;

using ReadEvtCallback = std::function<void(Connection*, Buffer*)>;
using WriteEvtCallback = std::function<void(Buffer*)>;

} // namespace ladder

#endif
