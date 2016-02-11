#pragma once

#include <map>
#include <vector>
#include <chrono>

namespace Network {

  class ConnectionController {
  public:
    void notify_pong(const std::string& ip);
    void remove_clients(const std::vector<std::string>& ips);
    std::vector<std::string> get_clients() const;
    bool has_client(const std::string& ip) const;
    void run();

  private:
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    const std::chrono::duration<double> timeout_limit = std::chrono::seconds(1);
    const std::chrono::duration<double> ping_period = std::chrono::milliseconds(500);
    //    const double timeout_limit = 1;
    //const double ping_period = 0.5;
    TimePoint last_ping;
    std::map<std::string, TimePoint> connections;
    void check_timeouts();
    void send_ping();
  };

  extern ConnectionController connection_controller;
}
