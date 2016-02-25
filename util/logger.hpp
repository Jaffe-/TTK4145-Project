#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <mutex>

#define LOG(Level_, Message_)		\
  if ((int)log_.include_level >= (int)(Level_))				\
    (log_.new_line().write((Logger::LogLevel)Level_, __FILE__, __FUNCTION__, __LINE__) << Message_ << std::endl)

#define LOG_DEBUG(Message_) LOG(Logger::LogLevel::DEBUG, Message_)
#define LOG_WARNING(Message_) LOG(Logger::LogLevel::WARNING, Message_)
#define LOG_INFO(Message_) LOG(Logger::LogLevel::INFO, Message_) 
#define LOG_ERROR(Message_) LOG(Logger::LogLevel::ERROR, Message_)

class Logger {
public:
  enum class LogLevel {
    ERROR, WARNING, INFO, DEBUG
  };
  Logger(std::string const& filename, LogLevel level);
  ~Logger();

  class Line {
  public:
    Line(Logger& parent);
    ~Line();
    std::ostream& write(LogLevel level, char const* filename, char const* function, int line);

  private:
    Logger& parent;
  };

  Line new_line();
  
  LogLevel include_level;

private:
  static std::string level_name(LogLevel level);
  std::ofstream file;
  std::mutex mut;
};

extern Logger log_;