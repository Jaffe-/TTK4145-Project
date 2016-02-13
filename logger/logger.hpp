#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>

#define LOG(Level_, Message_)		\
  if (log_.include_level >= (Level_)) \
    (log_.write(Level_, __FILE__, __FUNCTION__) << Message_ << std::endl)

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
  std::ostream& write(LogLevel level, char const* filename, char const* function);
  void new_context(const std::string& ctx);
  bool includes_context(const std::string& ctx) const;

  LogLevel include_level;

private:
  static std::string level_name(LogLevel level);
  std::ofstream file;
  std::vector<std::string> contexts;
};

extern Logger log_;
