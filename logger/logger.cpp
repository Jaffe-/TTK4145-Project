#include "logger.hpp"
#include <ctime>


std::ostream& Logger::write(LogLevel level, char const* filename, char const* function)
{
  std::time_t raw_time = std::time(NULL);
  char formatted_time[100];
  std::strftime(formatted_time, sizeof(formatted_time), "%m/%d %H:%M:%S", std::localtime(&raw_time));

  file << formatted_time << " <" << level_name(level) << "> " << filename
       << ":" << function << ":  ";
  return file;
}

bool Logger::includes_context(const std::string& ctx) const
{
  return std::find(contexts.begin(), contexts.end(), ctx) != contexts.end();
}

void Logger::new_context(const std::string& ctx)
{
  contexts.push_back(ctx);
}

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

Logger log_("tst.txt", Logger::LogLevel::DEBUG);
