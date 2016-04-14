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
  void notify(const FSMOrderCompleteEvent& event);
  void notify(const NetworkMessageEvent<OrderCompleteEvent>& event);
  void run();

private:

  EventQueue event_queue;
  Driver driver;
  Network network;

  struct ElevatorInfo {
    bool active;
    State state;
  };

  struct OrderInfo {
    int floor;
    int type;
    std::string owner;
  };

  std::map<std::string, ElevatorInfo> elevator_infos;
  std::map<std::string, OrderInfo> orders;

  std::thread driver_thread;
  std::thread network_thread;

  const std::string backup_filename = "orders.txt";

  int current_id = 0;

  void choose_elevator(const std::string& id, int floor, ButtonType type);
  bool restore_orders();
  void backup_orders();
};
