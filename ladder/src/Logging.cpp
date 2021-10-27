#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <Logging.h>
#include <utils.h>

namespace ladder {

std::string GetCurrentDateTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm* current = localtime(&(tv.tv_sec));
  char buf[40], minor_buf[10];
  strftime(buf, sizeof(buf), "%Y-%m-%d %X.", current);
  snprintf(minor_buf, sizeof(minor_buf), "%06ld", tv.tv_usec);
  strncat(buf, minor_buf, sizeof(buf) - strlen(buf) - 1);

  return std::string(buf);
}

Logger* Logger::instance() { return instance_ ? instance_ : Logger::create(); }

Logger* Logger::create(std::string log_path, int level) {
  static Logger* instance = new Logger(log_path.c_str(), level);
  return instance_ = instance;
}

void Logger::release() {
  if (instance_) delete instance_;
}

Logger* Logger::instance_ = nullptr;

Logger::Logger(const char* filename, int level)
    : running_(true), level_(level) {
  if (strlen(filename) == 0) {
    char log_path[20] = {0};
    snprintf(log_path, sizeof(log_path), "ladder.%d.log", getpid());
    filename = log_path;
  }
  fd_ = ::open(filename, O_CREAT | O_WRONLY | O_APPEND);
  if (fd_ < 0) {
    EXIT("[Logger] open");
  }
  logging_thread_ = std::thread(&Logger::ThreadFunc, this);
}

Logger::~Logger() {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    running_ = false;
  }
  condition_.notify_one();
  logging_thread_.join();
  close(fd_);
}

void Logger::WriteLogFmt(enum LogLevel severity, const char* fmt, ...) {
  if (static_cast<int>(severity) < level_) return;
  std::string prefix = "[" + GetCurrentDateTime() + "][" +
                       kLogLevels[static_cast<int>(severity)] + "] ";
  char msg[kMaxMessageLength];
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);
  std::string message(msg, msg + ret);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    message_queue_.emplace(prefix + message + "\n");
  }
  condition_.notify_one();
}

void Logger::ThreadFunc() {
  std::string msg;
  while (1) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.wait(
          lock, [this]() { return !running_ || !message_queue_.empty(); });
      if (!running_ && message_queue_.empty()) {
        break;
      }
      msg = message_queue_.front();
      message_queue_.pop();
    }
    int ret = ::write(fd_, msg.c_str(), msg.size());
    if (ret < -1) {
      // TODO: handle write error
    }
  }
}

const char* Logger::kLogLevels[] = {"TRACE",   "DEBUG", "INFO",
                                    "WARNING", "ERROR", "FATAL"};

}  // namespace ladder
