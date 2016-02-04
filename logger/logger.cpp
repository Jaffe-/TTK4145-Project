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
  }
}

Logger log("tst.txt", Logger::LogLevel::ERROR);

int main()
{
  LOG_DEBUG("Added 2 to i");
  LOG_WARNING("Fuck me, this may be wrong");
  LOG_ERROR("ERROR! DAMN IT TO HELL");
  std::cout << std::endl;
  return 0;
}
