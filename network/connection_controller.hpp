#pragma once
#include <map>
#include <ctime>

class ConnectionController {
public:
  void notify_pong(std::string ip);
  void run();
private:
  std::map<std::string, time_t> connections;
}
