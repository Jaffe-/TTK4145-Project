#include "connection_controller.hpp"
#include <vector>
#include "../logger/logger.hpp"

namespace Network {
  double ConnectionController::get_time()
  {
    return static_cast<double>(std::clock())/CLOCKS_PER_SEC;
  }

  void ConnectionController::notify_pong(std::string ip) 
  {
    connections[ip] = get_time();
  }

  void ConnectionController::check_timeouts()
  {
    std::vector<std::string> timed_out;
    for (auto& kv : connections) {
      double difference = get_time() - kv.second;
      if (difference > timeout_limit) {
	LOG_WARNING(kv.first << " stopped responding to PING");
	// notify the callback thing
	timed_out.push_back(kv.first);
      }
    }
    for (auto& ip : timed_out) {
      connections.erase(ip);
    }
  }

  void ConnectionController::send_ping()
  {
    double time = get_time();
    if (time - last_ping >= ping_period) {
      // send ping
      last_ping = time; 
    }
  }

  void ConnectionController::run()
  {
    check_timeouts();
    send_ping();
  }
}
