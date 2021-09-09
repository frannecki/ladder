#ifndef LADDER_LOGGER_H
#define LADDER_LOGGER_H

#include <mutex>
#include <string>
#include <queue>
#include <thread>

namespace ladder {

#define LOG_FATAL(message) LOG_SEVERITY(message, FATAL)
#define LOG_WARNING(message) LOG_SEVERITY(message, WARNING)
#ifdef DEBUG
  // #define LOG_DEBUG(message) fprintf(stderr, "[%s:%u %s] ", __FILE__, __LINE__, __FUNCTION__); LOG_SEVERITY(message, DEBUG)
  #define LOG_DEBUG(message) LOG_SEVERITY(message, DEBUG)
#else
  #define LOG_DEBUG(message)
#endif
#define LOG_INFO(message) LOG_SEVERITY(message, INFO)
#define LOG_SEVERITY(message, severity) Logger::instance()->WriteLog(message, #severity)

std::string GetCurrentDateTime();

class Logger {

public:
  static Logger* instance();
  static Logger* create(std::string log_path = "");
  static void release();

  template <typename MessageType>
  void WriteLog(MessageType&& message, std::string&& severity) {
    if(this == nullptr) {
      return;
    }
    std::string line = "[" + GetCurrentDateTime() + "][" + severity + "] " + std::forward<MessageType>(message) + "\n";
    // {
      // std::lock_guard<std::mutex> lock(mutex_);
      message_queue_.emplace(line);
    // }
  }

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
