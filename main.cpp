#include "util/cmdopts.hpp"
#include "util/logger.hpp"
#include "network/network.hpp"
#include "driver/driver.hpp"
#include <thread>

class NetworkMessage : public Message, public Serializable {
public:
  bool serializable() const override { return true; };
  std::string data;
  int id;

  NetworkMessage(std::string d, int i) : data(d), id(i) {};
  NetworkMessage(const std::string& s) {
    //    deserialize(s);
  }
  
  json_t get_json() const override {
    return {{"id", id}, {"data", data}};
  }

  //  void deserialize(const std::string& s) override {};
};

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

  Network network(cmd_options["port"]);
  Driver driver(cmd_options.has("simulated"));

  std::thread driver_thread([&] {
      driver.run();
  });

  std::thread network_thread([&] {
      network.run();
  });

  TimePoint t = std::chrono::system_clock::now();
  while (1) {
    if (std::chrono::system_clock::now() - t > std::chrono::seconds(2)) {
      NetworkMessage m {"Test!", 100};
      network.message_queue.push(m);
      t = std::chrono::system_clock::now();
    }
  };
}
