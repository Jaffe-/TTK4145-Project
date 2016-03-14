#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <mutex>
#include <thread>

#define LOG(Level_, Message_)		\
  if ((int)get_logger().include_level >= (int)(Level_))			\
    (get_logger().new_line().write((Logger::LogLevel)Level_, __FILE__, __FUNCTION__, __LINE__) << Message_ << std::endl)

#define LOG_DEBUG(Message_) LOG(Logger::LogLevel::DEBUG, Message_)
#define LOG_WARNING(Message_) LOG(Logger::LogLevel::WARNING, Message_)
#define LOG_INFO(Message_) LOG(Logger::LogLevel::INFO, Message_) 
#define LOG_ERROR(Message_) LOG(Logger::LogLevel::ERROR, Message_)

class Logger {
public:
  enum class LogLevel {
    ERROR, WARNING, INFO, DEBUG
  };
  Logger() : include_level(LogLevel::ERROR) {};
  Logger(std::string const& filename) : include_level(LogLevel::ERROR),
					file(filename) {};

  class Line {
  public:
    Line(Logger& log, std::unique_lock<std::mutex> lock)
      : log(log),
	lock(std::move(lock)) {};

    Line(Line&& line) : log(line.log),
			lock(std::move(line.lock)) {};

    std::ostream& write(LogLevel level, char const* filename, char const* function, int line);

  private:
    Logger& log;
    std::unique_lock<std::mutex> lock;
  };

  Line new_line();
  
  void set_level(LogLevel level) {
    include_level = level;
  }

  LogLevel include_level;
private:
  static std::string level_name(LogLevel level);
  std::ofstream file;
  std::mutex mut;
};

Logger& get_logger();
