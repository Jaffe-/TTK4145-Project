#include "cmdopts/cmdopts.hpp"
#include "network/receiver.hpp"
#include "logger/logger.hpp"

int main(int argc, char** argv)
{
  std::cout << "******************************************************" << std::endl
	    << "*                                                    *" << std::endl
	    << "*         Voll Electronics UltraElevator4000         *" << std::endl
	    << "*                                                    *" << std::endl
	    << "******************************************************" << std::endl << std::endl
	    << "\"Now I don't have to get stuck in elevators any more!\"" << std::endl
	    << "                                          - Anonymous" << std::endl << std::endl;

  CmdOptions cmd_options;
  if (!cmd_options_get(cmd_options, argc, argv,
		       {{"port", true, false, "port", "The port used for sending and receiving"},
			{"debug", true, true, "debug", "Include debug information in log"}})) {
    return -1;
  }

  if (cmd_options.find("debug") == cmd_options.end()) {
    log_.include_level = Logger::LogLevel::WARNING;
  }
  else {
    log_.include_level = static_cast<Logger::LogLevel>(std::stoi(cmd_options["debug"]));
  }
  LOG_DEBUG("Log is initialized");
  Network::start(cmd_options["port"]);
  Network::run();
}
