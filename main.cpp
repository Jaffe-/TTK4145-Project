#include "util/cmdopts.hpp"
#include "util/logger.hpp"
#include "logic/logic.hpp"
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
			  {"stdout", true, true, "level", "Print log messages up to the given level to stdout"},
			  {"log-level", true, true, "level", "Explicitly set level to include in log (default is all levels)"},
			  {"simulated", false, true, "simulated", "Use elevator simulator instead of real hardware"}});

  get_logger().use_stdout = cmd_options.has("stdout");
  if (get_logger().use_stdout)
    get_logger().stdout_level = string_to_loglevel(cmd_options["stdout"]);

  if (cmd_options.has("log-level"))
    get_logger().log_level = string_to_loglevel(cmd_options["log-level"]);

  LOG_DEBUG("Log is initialized");

  DispatchLogic dispatch_logic(cmd_options.has("simulated"), cmd_options["port"]);

  dispatch_logic.run();
}
