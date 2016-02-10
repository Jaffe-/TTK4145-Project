#pragma once

#include <map>
#include <vector>

namespace Network {

  class ConnectionController {
  public:
    void notify_pong(std::string ip);
    void run();
    std::vector<std::string> get_clients();
    void remove_clients(const std::vector<std::string> ips);
    bool has_client(std::string ip);
  private:
    const double timeout_limit = 1;
    const double ping_period = 0.5;
    double last_ping;
    std::map<std::string, double> connections;
    void check_timeouts();
    void send_ping();
  };

  extern ConnectionController connection_controller;
}
