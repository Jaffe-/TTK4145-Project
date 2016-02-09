#pragma once

#include <iostream>
#include <fstream>
#include <ctime>

#define LOG(Level_, Message_)					\
  if (log_.include_level >= (Level_))					\
    (Logger::LogLine(log_, Level_, __FILE__, __FUNCTION__) << Message_ << std::endl)

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
    std::ostream& operator<<(const T& msg) {
      std::time_t raw_time = std::time(NULL);
      char formatted_time[100];
      std::strftime(formatted_time, sizeof(formatted_time), "%m/%d %H:%M:%S", std::localtime(&raw_time));
      
      log.file << formatted_time << " <" << level_name(level) << "> " << file << ":" << function << ":  " << msg;
      return log.file;
    }

    Logger& log;
    LogLevel level;
    char const* file;
    char const* function;
  };

  LogLevel include_level;

private:
  static std::string level_name(LogLevel level);
  std::ofstream file;
};

extern Logger log_;
