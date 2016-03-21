#pragma once
#include "../util/event_queue.hpp"
#include "../driver/driver.hpp"
#include "../network/network.hpp"
#include "../network/network_events.hpp"
#include <thread>

class Logic {
public:
  Logic(bool use_simulator, const std::string& port);
  void run();

private:
  void notify(const StateUpdateEvent& event);
  void notify(const NetworkReceiveEvent<StateUpdateEvent>& event);
  void notify(const NetworkReceiveEvent<ExternalButtonEvent>& event);
  void notify(const LostConnectionEvent& event);

  EventQueue event_queue;
  Driver driver;
  Network network;

  std::map<std::string, State> elevator_states;

  std::thread driver_thread;
  std::thread network_thread;
};
