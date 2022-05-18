#include <functional>
#include <iostream>

#include <EventLoop.h>
#include <Logging.h>
#include <Timer.h>

static int tick_ = 0;
static int tick1_ = 0;

void PrintTick(int *tick) {
  std::cout << "Current tick is " << *tick << std::endl;
  *tick += 1;
}

void PrintTick1(int *tick) {
  std::cout << "Current tick1 is " << *tick << std::endl;
  *tick += 1;
}

int main(int argc, char **argv) {
#ifdef LADDER_OS_WINDOWS
  ladder::Timer *timer = new ladder::Timer;
  ladder::Timer *timer1 = new ladder::Timer;
#else
  ladder::EventLoop loop;
  ladder::Timer *timer = new ladder::Timer(ladder::EventLoopPtr(&loop));
  ladder::Timer *timer1 = new ladder::Timer(ladder::EventLoopPtr(&loop));
#endif
  timer->SetTimmerEventCallback(std::bind(PrintTick, &tick_));
  timer1->SetTimmerEventCallback(std::bind(PrintTick1, &tick1_));
#ifndef LADDER_HAVE_KQUEUE
  timer->SetInterval(1000000);  // triggered once
#endif
  timer1->SetInterval(1000000, true);  // triggered periodically
#ifdef LADDER_OS_WINDOWS
  while (1)
    ;
#else
  loop.StartLoop();
#endif
  return 0;
}
