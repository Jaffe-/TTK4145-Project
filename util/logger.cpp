#include "logger.hpp"
#include <ctime>

const std::string color_red = "\x1b[31m";
const std::string color_green = "\x1b[32m";
const std::string color_yellow = "\x1b[33m";
const std::string color_cyan = "\x1b[96m";
const std::string color_darkcyan = "\x1b[36m";
const std::string color_white = "\x1b[97m";
const std::string color_off = "\x1b[0m";
const std::string color_darkblue = "\x1b[34m";
const std::string color_blue = "\x1b[94m";

std::string line_color(Logger::LogLevel level)
{
  switch (level) {
  case Logger::LogLevel::ERROR:
    return color_red;
  case Logger::LogLevel::WARNING:
    return color_yellow;
  case Logger::LogLevel::INFO:
    return color_green;
  default:
    return color_off;
  }
}

Logger::Line Logger::new_line()
{
  return Line(*this);
}

Logger::Line::~Line()
{
  parent.write(level, ss.str());
}

std::ostream& Logger::Line::write(LogLevel level, char const* filename, char const* function, int line)
{
  this->level = level;
  std::time_t raw_time = std::time(NULL);
  char formatted_time[100];
  std::strftime(formatted_time, sizeof(formatted_time), "%m/%d %H:%M:%S", std::localtime(&raw_time));

  std::string color = line_color(level);

  ss << color << formatted_time << " "
     << color_darkblue << filename
     << color_white << ":"
     << color_blue << function
     << color_white << ":"
     << color_darkcyan << line
     << color_white << ": "
     << color;
  return ss;
}

void Logger::write(LogLevel level, const std::string& logline)
{
  std::unique_lock<std::mutex>(mut);
  if (static_cast<int>(level) <= static_cast<int>(log_level)) {
    file << logline;
    file.flush();
  }
  if (use_stdout && static_cast<int>(level) <= static_cast<int>(stdout_level))
    std::cout << logline;
}

std::string Logger::level_name(Logger::LogLevel level)
{
  switch (level) {
  case LogLevel::WARNING:
    return "WARNING";
  case LogLevel::ERROR:
    return "ERROR";
  case LogLevel::INFO:
    return "INFO";
  default:
    return "DEBUG";
  }
}

Logger& get_logger() {
  static Logger logger("log.txt");
  return logger;
}

Logger::LogLevel string_to_loglevel(const std::string& s)
{
  return static_cast<Logger::LogLevel>(std::stoi(s));
}
