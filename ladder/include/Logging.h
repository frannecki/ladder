#ifndef LADDER_LOGGING_H
#define LADDER_LOGGING_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "Base.h"

namespace ladder {

const int kMaxMessageLength = 1024;

enum LADDER_API LogLevel : int {
  kLogTrace = 0,
  kLogDebug,
  kLogInfo,
  kLogWarning,
  kLogError,
  kLogFatal
};

#define LOG_FATAL(message) LOG_SEVERITY(message, LogLevel::kLogFatal)
#define LOG_ERROR(message) LOG_SEVERITY(message, LogLevel::kLogError)
#define LOG_WARNING(message) LOG_SEVERITY(message, LogLevel::kLogWarning)
#define LOG_INFO(message) LOG_SEVERITY(message, LogLevel::kLogInfo)
#define LOG_DEBUG(message) LOG_SEVERITY(message, LogLevel::kLogDebug)
#define LOG_TRACE(message) LOG_SEVERITY(message, LogLevel::kLogTrace)
#define LOG_SEVERITY(message, severity) \
  if (Logger::instance()) Logger::instance()->WriteLog(message, severity)

// for formatted logging
#define LOGF_FATAL(fmt, ...) \
  LOGF_SEVERITY(fmt, LogLevel::kLogFatal, __VA_ARGS__)
#define LOGF_ERROR(fmt, ...) \
  LOGF_SEVERITY(fmt, LogLevel::kLogError, __VA_ARGS__)
#define LOGF_WARNING(fmt, ...) \
  LOGF_SEVERITY(fmt, LogLevel::kLogWarning, __VA_ARGS__)
#define LOGF_INFO(fmt, ...) LOGF_SEVERITY(fmt, LogLevel::kLogInfo, __VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) \
  LOGF_SEVERITY(fmt, LogLevel::kLogDebug, __VA_ARGS__)
#define LOGF_TRACE(fmt, ...) \
  LOGF_SEVERITY(fmt, LogLevel::kLogTrace, __VA_ARGS__)
#define LOGF_SEVERITY(fmt, severity, ...) \
  if (Logger::instance())                 \
  Logger::instance()->WriteLogFmt(severity, fmt, __VA_ARGS__)

LADDER_API std::string GetCurrentDateTime();

class LADDER_API Logger {
 public:
  static Logger* instance();

  /*
   * Log levels:
   * 0 - Trace
   * 1 - Debug
   * 2 - Info
   * 3 - Warning
   * 4 - Error
   * 5 - Fatal
   */
  static Logger* create(std::string log_path = "",
                        int level = static_cast<int>(LogLevel::kLogDebug));
  static void release();

  template <typename MessageType>
  void WriteLog(MessageType&& message, enum LogLevel severity) {
    if (static_cast<int>(severity) < level_) return;
    std::string line = "[" + GetCurrentDateTime() + "][" +
                       kLogLevels[static_cast<int>(severity)] + "] " +
                       std::forward<MessageType>(message) + "\n";
    {
      std::unique_lock<std::mutex> lock(mutex_);
      message_queue_.emplace(line);
    }
    condition_.notify_one();
  }

  void WriteLogFmt(enum LogLevel severity, const char* fmt, ...);

 private:
  Logger(const char* filepath, int level);
  ~Logger();
  void ThreadFunc();
  FILE* fp_;
  bool running_;
  int level_;
  // std::mutex mutex_;
  std::queue<std::string> message_queue_;
  std::mutex mutex_;
  std::thread logging_thread_;
  std::condition_variable condition_;
  static Logger* instance_;

  static const char* kLogLevels[];
};

}  // namespace ladder

#endif
