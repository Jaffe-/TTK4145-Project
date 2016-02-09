#include "network.hpp"
#include "connection_controller.hpp"
#include <vector>
#include "../logger/logger.hpp"
#include "sender.hpp"
#include <algorithm>

namespace Network {

  void ConnectionController::notify_pong(std::string ip) 
  {
    if (std::find_if(connections.begin(), connections.end(),
		     [&] (std::pair<std::string, double> p) { return p.first == ip; })
	== connections.end()) {
      // new client registered, send event
      LOG_DEBUG("New client " << ip << " discovered");
    }
    connections[ip] = get_time();
  }

  void ConnectionController::remove_clients(const std::vector<std::string> ips)
  {
    for (auto& ip : ips) {
      connections.erase(ip);
    }
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

    remove_clients(timed_out);
  }

  void ConnectionController::send_ping()
  {
    double time = get_time();
    if (time - last_ping >= ping_period) {
      sender->broadcast({PacketType::PING, {}, ""});
      last_ping = time; 
    }
  }

  void ConnectionController::run()
  {
    check_timeouts();
    send_ping();
  }

  std::vector<std::string> ConnectionController::get_clients()
  {
    std::vector<std::string> results;

    for (auto& kv: connections) {
      results.push_back(kv.first);
    }

    return results;
  }

  ConnectionController connection_controller;
}
