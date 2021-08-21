#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <Logger.h>

namespace ladder {

std::string GetCurrentDateTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm  *current = localtime(&(tv.tv_sec));
    char buf[40], minor_buf[10];
    strftime(buf, sizeof(buf), "%Y-%m-%d %X.", current);
    snprintf(minor_buf, sizeof(minor_buf), "%06u", tv.tv_usec);
    strncat(buf, minor_buf, sizeof(buf));

    return std::string(buf);
}

Logger* Logger::instance() {
  if(instance == nullptr) {
    fprintf(stderr, "[Logger] Logger instance not created!");
    exit(-1);
  }
  return instance_;
}

Logger* Logger::create(std::string log_path) {
  static Logger* instance = new Logger(log_path.c_str());
  return instance_ = instance;
}

void Logger::release() {
  delete instance_;
}

Logger* Logger::instance_ = nullptr;

Logger::Logger(const char* filename) : running_(true) {
  if(strlen(filename) == 0) {
    fd_ = STDERR_FILENO;
  }
  else {
    fd_ = ::open(filename,
                 O_CREAT | O_WRONLY | O_APPEND | O_NONBLOCK);
    if(fd_ < 0) {
      perror("[Logger] open");
      exit(-1);
    }
  }
  logging_thread_ = std::thread(&Logger::ThreadFunc, this);
}

Logger::~Logger() {
  // {
  //  std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
  // }
  logging_thread_.join();
  close(fd_);
}

void Logger::ThreadFunc() {
  while(running_ || !message_queue_.empty()) {
    if(message_queue_.empty()) {
      continue;
    }
    std::string msg = message_queue_.front();
    int ret = ::write(fd_, msg.c_str(), msg.size());
    if(ret < -1) {
      // TODO: handle write error
    }
    else {
      // std::lock_guard<std::mutex> lock(mutex_);
      message_queue_.pop();
    }
  }
}

} // namespace ladder
