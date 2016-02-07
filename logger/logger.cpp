#include "logger.hpp"
#include <ctime>

Logger::Logger(std::string const& filename, Logger::LogLevel level) : include_level(level)
{
  file.open(filename);
}

Logger::~Logger()
{
  file.close();
}

std::string Logger::level_name(Logger::LogLevel level)
{
  switch (level) {
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::WARNING:
    return "WARNING";
  case LogLevel::ERROR:
    return "ERROR";
  default:
    return "<undefined>";
  }
}

Logger log("tst.txt", Logger::LogLevel::DEBUG);
