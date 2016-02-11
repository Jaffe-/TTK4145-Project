#pragma once

#include <map>
#include <vector>

namespace Network {

  class ConnectionController {
  public:
    void notify_pong(const std::string& ip);
    void remove_clients(const std::vector<std::string>& ips);
    std::vector<std::string> get_clients() const;
    bool has_client(const std::string& ip) const;
    void run();

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
