#pragma once
#include <map>
#include <ctime>

namespace Network {
  class ConnectionController {
  public:
    void notify_pong(std::string ip);
    void run();
  private:
    const double timeout_limit = 0.5;
    const double ping_period = 0.1;
    double last_ping;
    std::map<std::string, double> connections;
    double get_time();
    void check_timeouts();
    void send_ping();
  };
}
