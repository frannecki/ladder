#include <time.h>
#ifdef __unix__
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include <chrono>
#include <ctime>

#include <Logger.h>
#include <utils.h>

namespace ladder {

std::string GetCurrentDateTime() {
    char buf[40], minor_buf[10];
#ifdef __unix__
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm  *current = localtime(&(tv.tv_sec));
    strftime(buf, sizeof(buf), "%Y-%m-%d %X.", current);
    snprintf(minor_buf, sizeof(minor_buf), "%06ld", tv.tv_usec);
#endif
#ifdef _MSC_VER
    auto current = std::chrono::system_clock::now();
    auto cur_ms = std::chrono::duration_cast<std::chrono::microseconds>(
        current.time_since_epoch()) % 1000000;
    std::time_t cur_time_t = std::chrono::system_clock::to_time_t(current);
    std::tm* cur_tm = std::localtime(&cur_time_t);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.", cur_tm);
#endif
    int ret = snprintf(minor_buf, sizeof(minor_buf), "%06u",
                       static_cast<uint32_t>(cur_ms.count()));
    strncat(buf, minor_buf, sizeof(buf));

    return std::string(buf);
}

Logger* Logger::instance() {
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
    fp_ = stdout;
  }
  else {
    fp_ = ::fopen(filename, "a");
    if (fp_ == NULL) {
        EXIT("[Logger] open");
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
  fclose(fp_);
}

void Logger::ThreadFunc() {
  while(running_ || !message_queue_.empty()) {
    if(message_queue_.empty()) {
      continue;
    }
    std::string msg = message_queue_.front();
    int ret = fprintf(fp_, msg.c_str(), msg.size());
    if(ret < 0) {
      // TODO: handle write error
    }
    else {
      // std::lock_guard<std::mutex> lock(mutex_);
      message_queue_.pop();
    }
  }
}

} // namespace ladder
