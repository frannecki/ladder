#ifndef LADDER_UTILS_H
#define LADDER_UTILS_H

#include <stdio.h>

namespace ladder {

const int kEnableOption = 1;

#define EXIT fprintf(stderr, "[%s:%u %s] ", __FILE__, __LINE__, __FUNCTION__); exit_fatal

void exit_fatal(const char* msg);


} // namespace ladder

#endif
