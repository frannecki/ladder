#include <stdio.h>
#include <stdlib.h>

#include <utils.h>
namespace ladder {

void exit_fatal(const char* msg) {
  perror(msg);
  exit(-1);
}

} // namespace ladder
