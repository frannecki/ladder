#ifndef LADDER_LOGGER_H
#define LADDER_LOGGER_H

#include <mutex>
#include <string>
#include <queue>
#include <thread>

#include <iostream>

namespace ladder {

const int kMaxMessageLength = 1024;

enum LogLevel : int {
  kLogTrace = 0,
  kLogDebug,
  kLogInfo,
  kLogWarning,
  kLogError,
  kLogFatal
};

#define LOG_FATAL(message) LOG_SEVERITY(message, kLogFatal)
#define LOG_ERROR(message) LOG_SEVERITY(message, kLogError)
#define LOG_WARNING(message) LOG_SEVERITY(message, kLogWarning)
#define LOG_INFO(message) LOG_SEVERITY(message, kLogInfo)
#ifdef DEBUG
  #define LOG_DEBUG(message) LOG_SEVERITY(message, kLogDebug)
#else
  #define LOG_DEBUG(message)
#endif
#define LOG_TRACE(message) LOG_SEVERITY(message, kLogTrace)
#define LOG_SEVERITY(message, severity) if(Logger::instance()) Logger::instance()->WriteLog(message, severity)

// for formatted logging
#define LOGF_FATAL(fmt, ...) LOGF_SEVERITY(fmt, kLogFatal, __VA_ARGS__)
#define LOGF_ERROR(fmt, ...) LOGF_SEVERITY(fmt, kLogError, __VA_ARGS__)
#define LOGF_WARNING(fmt, ...) LOGF_SEVERITY(fmt, kLogWarning, __VA_ARGS__)
#define LOGF_INFO(fmt, ...) LOGF_SEVERITY(fmt, kLogInfo, __VA_ARGS__)
#ifdef DEBUG
  #define LOGF_DEBUG(fmt, ...) LOGF_SEVERITY(fmt, kLogDebug, __VA_ARGS__)
#else
  #define LOGF_DEBUG(fmt, ...)
#endif
#define LOGF_TRACE(fmt, ...) LOGF_SEVERITY(fmt, kLogTrace, __VA_ARGS__)
#define LOGF_SEVERITY(fmt, severity, ...) if(Logger::instance()) Logger::instance()->WriteLogFmt(severity, fmt, __VA_ARGS__)

std::string GetCurrentDateTime();

class Logger {

public:
  static Logger* instance();
  static Logger* create(std::string log_path = "",
                        int level = static_cast<int>(kLogDebug));
  static void release();

  template <typename MessageType>
  void WriteLog(MessageType&& message, enum LogLevel severity) {
    if(static_cast<int>(severity) < level_)  return;
    std::string line = "[" + GetCurrentDateTime() + "][" + kLogLevels[static_cast<int>(severity)] + \
                       "] " + std::forward<MessageType>(message) + "\n";
    // {
      // std::lock_guard<std::mutex> lock(mutex_);
      message_queue_.emplace(line);
    // }
  }

  void WriteLogFmt(enum LogLevel severity, const char* fmt, ...);

private:
  Logger(const char* filepath, int level);
  ~Logger();
  void ThreadFunc();

  int fd_;
  bool running_;
  int level_;
  // std::mutex mutex_;
  std::queue<std::string> message_queue_;
  std::thread logging_thread_;
  static Logger* instance_;

  static const char* kLogLevels[];
};

} // namespace ladder

#endif
