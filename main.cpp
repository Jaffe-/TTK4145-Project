#include "cmdopts/cmdopts.hpp"
#include "network/receiver.hpp"

int main(int argc, char** argv)
{
  CmdOptions cmd_options;
  if (!cmd_options_get(cmd_options, argc, argv,
		       {{"port", 1, "<port>", "The port used for sending and receiving"}})) {
    return -1;
  }

  Network::start("30021");
  Network::receiver->run();
}
