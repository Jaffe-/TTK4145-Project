#pragma once

#include <map>
#include <vector>
#include <chrono>
#include "network.hpp"

class Network;

class ConnectionController {
public:
  ConnectionController(Network& network) : network(network) {};
  void notify_pong(const std::string& ip);
  void remove_clients(const std::vector<std::string>& ips);
  std::vector<std::string> get_clients() const;
  bool has_client(const std::string& ip) const;
  void run();

private:
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
  
  const std::chrono::duration<double> timeout_limit = std::chrono::seconds(1);
  const std::chrono::duration<double> ping_period = std::chrono::milliseconds(200);
  TimePoint last_ping;
  Network& network;
  
  void check_timeouts();
  void send_ping();
};
