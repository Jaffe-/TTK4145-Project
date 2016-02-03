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

template<typename T>
std::ostream& Logger::LogLine::operator<<(const T& msg)
{
  std::time_t raw_time = std::time(NULL);
  char formatted_time[100];
  std::strftime(formatted_time, sizeof(formatted_time), "%j/%d %H:%M:%S", std::localtime(&raw_time));

  log.file << formatted_time << "  <" << level_name(level) << ">  " << file << ":" << function << ":  " << msg << std::endl;
  return log.file;
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
  }
}

Logger log("tst.txt", Logger::LogLevel::DEBUG);

int main()
{
  LOG_DEBUG("Added 2 to i");
  LOG_WARNING("Fuck me, this may be wrong");
  LOG_ERROR("ERROR! DAMN IT TO HELL");
  std::cout << std::endl;
  return 0;
}
