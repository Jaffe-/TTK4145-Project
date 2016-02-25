#include "util/cmdopts.hpp"
#include "util/logger.hpp"
#include "network/receiver.hpp"
#include "driver/driver.hpp"
#include <thread>

int main(int argc, char** argv)
{
  std::cout << "******************************************************" << std::endl
	    << "*                                                    *" << std::endl
	    << "*         Voll Electronics UltraElevator4000         *" << std::endl
	    << "*                                                    *" << std::endl
	    << "******************************************************" << std::endl << std::endl
	    << "\"Now I don't have to get stuck in elevators any more!\"" << std::endl
	    << "                                          - Anonymous" << std::endl << std::endl;

  CmdOptions cmd_options(argc, argv,
			 {{"port", true, false, "port", "The port used for sending and receiving"},
			  {"debug", true, true, "debug", "Include debug information in log"},
			  {"simulated", false, true, "simulated", "Use elevator simulator instead of real hardware"}});

  if (!cmd_options.has("debug")) {
    log_.include_level = Logger::LogLevel::INFO;
  }
  else {
    log_.include_level = static_cast<Logger::LogLevel>(std::stoi(cmd_options["debug"]));
  }
  LOG_DEBUG("Log is initialized");

  Driver driver(cmd_options.has("simulated"));
  Network::start(cmd_options["port"]);
  driver.run();
  std::thread driver_thread([&] { driver.run(); });
  std::thread network_thread(Network::run);

  while (1);
  //Network::run();
}
