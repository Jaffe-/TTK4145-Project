#pragma once

#include <iostream>
#include <fstream>

#define LOG(Level_, Message_)					\
  Logger::LogLine(log, Level_, __FILE__, __FUNCTION__) << Message_

#define LOG_DEBUG(Message_) LOG(Logger::LogLevel::DEBUG, Message_)
#define LOG_WARNING(Message_) LOG(Logger::LogLevel::WARNING, Message_)
#define LOG_ERROR(Message_) LOG(Logger::LogLevel::ERROR, Message_)

class Logger {
public:
  enum class LogLevel {
    ERROR, WARNING, DEBUG
  };
  Logger(std::string const& filename, LogLevel level);
  ~Logger();

  struct LogLine {
    LogLine(Logger& log, LogLevel level, char const* file, char const* function) :
      log(log), level(level), file(file), function(function) {};

    template<typename T>
    std::ostream& operator<<(const T& rhs);

    Logger& log;
    LogLevel level;
    char const* file;
    char const* function;
  };

private:
  static std::string level_name(LogLevel level);
  std::ofstream file;
  LogLevel include_level;
};

