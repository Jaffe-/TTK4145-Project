#pragma once
#include "../util/event_queue.hpp"
#include "../driver/driver.hpp"
#include "../network/network.hpp"
#include "../network/events.hpp"
#include "events.hpp"
#include <thread>
#include <vector>

class Logic {
public:
  Logic(bool use_simulator, const std::string& port);

  void notify(const ExternalButtonEvent& event);
  void notify(const StateUpdateEvent& event);
  void notify(const NetworkMessageEvent<StateUpdateEvent>& event);
  void notify(const NetworkMessageEvent<ExternalButtonEvent>& event);
  void notify(const LostConnectionEvent& event);
  void notify(const NewConnectionEvent&);
  void notify(const LostNetworkEvent&);
  void notify(const NetworkMessageEvent<StateUpdateReqEvent>& event);
  void choose_elevator(int floor, ButtonType type);
  void run();

private:

  EventQueue event_queue;
  Driver driver;
  Network network;

  struct ElevatorInfo {
    bool active;
    State state;
  };

  std::map<std::string, ElevatorInfo> elevator_infos;

  std::thread driver_thread;
  std::thread network_thread;

  const std::string backup_filename = "orders.txt";
  
  bool restore_orders(std::vector<std::vector<bool>>& orders);
  void backup_orders(const std::vector<std::vector<bool>>& orders);
};
