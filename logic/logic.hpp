#pragma once
#include "../util/message_queue.hpp"
#include "../driver/driver.hpp"
#include "../network/network.hpp"
#include <thread>

class Logic {
public:
  Logic(bool use_simulator, const std::string& port);
  void run();

private:
  MessageQueue message_queue;
  Driver driver;
  Network network;

  std::thread driver_thread;
  std::thread network_thread;
};
