#ifndef LADDER_LOGGER_H
#define LADDER_LOGGER_H

#include <mutex>
#include <string>
#include <queue>
#include <thread>

#include <iostream>

namespace ladder {

const int kMaxMessageLength = 1024;

#define LOG_FATAL(message) LOG_SEVERITY(message, FATAL)
#define LOG_WARNING(message) LOG_SEVERITY(message, WARNING)
#ifdef DEBUG
  #define LOG_DEBUG(message) LOG_SEVERITY(message, DEBUG)
#else
  #define LOG_DEBUG(message)
#endif
#define LOG_INFO(message) LOG_SEVERITY(message, INFO)
#define LOG_SEVERITY(message, severity) if(Logger::instance()) Logger::instance()->WriteLog(message, #severity)

// for formatted logging
#define LOGF_FATAL(fmt, ...) LOGF_SEVERITY(fmt, FATAL, __VA_ARGS__)
#define LOGF_WARNING(fmt, ...) LOGF_SEVERITY(fmt, WARNING, __VA_ARGS__)
#ifdef DEBUG
  #define LOGF_DEBUG(fmt, ...) LOGF_SEVERITY(fmt, DEBUG, __VA_ARGS__)
#else
  #define LOGF_DEBUG(fmt, ...)
#endif
#define LOGF_INFO(fmt, ...) LOGF_SEVERITY(fmt, INFO, __VA_ARGS__)
#define LOGF_SEVERITY(fmt, severity, ...) if(Logger::instance()) Logger::instance()->WriteLogFmt(#severity, fmt, __VA_ARGS__)

std::string GetCurrentDateTime();

class Logger {

public:
  static Logger* instance();
  static Logger* create(std::string log_path = "");
  static void release();

  template <typename MessageType>
  void WriteLog(MessageType&& message, std::string&& severity) {
    std::string line = "[" + GetCurrentDateTime() + "][" + severity + "] " + std::forward<MessageType>(message) + "\n";
    // {
      // std::lock_guard<std::mutex> lock(mutex_);
      message_queue_.emplace(line);
    // }
  }

  void WriteLogFmt(std::string&& severity, const char* fmt, ...);

private:
  Logger(const char* filepath);
  ~Logger();
  void ThreadFunc();

  int fd_;
  bool running_;
  // std::mutex mutex_;
  std::queue<std::string> message_queue_;
  std::thread logging_thread_;
  static Logger* instance_;
};

} // namespace ladder

#endif
