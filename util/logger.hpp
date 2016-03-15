#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <mutex>
#include <thread>
#include <sstream>

#define LOG(Level_, Message_)		\
  get_logger().new_line().write((Logger::LogLevel)Level_, __FILE__, __FUNCTION__, __LINE__) << Message_ << std::endl

#define LOG_DEBUG(Message_) LOG(Logger::LogLevel::DEBUG, Message_)
#define LOG_WARNING(Message_) LOG(Logger::LogLevel::WARNING, Message_)
#define LOG_INFO(Message_) LOG(Logger::LogLevel::INFO, Message_) 
#define LOG_ERROR(Message_) LOG(Logger::LogLevel::ERROR, Message_)

class Logger {
public:
  enum class LogLevel {
    ERROR, WARNING, INFO, DEBUG, DEBUG2, DEBUG3
  };
  Logger(std::string const& filename) : log_level(LogLevel::DEBUG3),
					file(filename) {};

  class Line {
  public:
    Line(Logger& p) : parent(p) {};
    Line(Line&& l) : parent(l.parent), ss(std::move(l.ss)) {};
    ~Line();
    std::ostream& write(LogLevel level, char const* filename, char const* function, int line);

  private:
    LogLevel level;
    Logger& parent;
    std::stringstream ss;
  };

  Line new_line();

  void write(LogLevel level, const std::string& logline);

  LogLevel log_level;
  LogLevel stdout_level;
  bool use_stdout;

private:
  static std::string level_name(LogLevel level);
  std::ofstream file;
  std::mutex mut;
};

Logger& get_logger();

Logger::LogLevel string_to_loglevel(const std::string& s);
