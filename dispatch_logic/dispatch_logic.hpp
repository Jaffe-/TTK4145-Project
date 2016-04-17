#pragma once
#include "../util/event_queue.hpp"
#include "../driver/driver.hpp"
#include "../network/network.hpp"
#include "../network/events.hpp"
#include "events.hpp"
#include <thread>
#include <vector>


class DispatchLogic {
public:
  DispatchLogic(bool use_simulator, const std::string& port);

  void notify(const ExternalButtonEvent& event);
  void notify(const StateUpdateEvent& event);
  void notify(const NetworkMessageEvent<StateUpdateEvent>& event);
  void notify(const NetworkMessageEvent<NewOrderEvent>& event);
  void notify(const LostConnectionEvent& event);
  void notify(const NewConnectionEvent&);
  void notify(const NetworkMessageEvent<UpdateRequestEvent>& event);
  void notify(const FSMOrderCompleteEvent& event);
  void notify(const NetworkMessageEvent<OrderCompleteEvent>& event);
  void notify(const NetworkMessageEvent<OrderMapUpdateEvent>& event);

  void run();

private:

  /* The events handled by this module */
  EventList<ExternalButtonEvent,
	    StateUpdateEvent,
	    NetworkMessageEvent<StateUpdateEvent>,
	    NetworkMessageEvent<NewOrderEvent>,
	    NewConnectionEvent,
	    LostConnectionEvent,
	    NetworkMessageEvent<UpdateRequestEvent>,
	    FSMOrderCompleteEvent,
	    NetworkMessageEvent<OrderCompleteEvent>,
	    NetworkMessageEvent<OrderMapUpdateEvent>> events;

  EventQueue event_queue;
  Driver driver;
  Network network;

  struct ElevatorInfo {
    bool active;
    State state;
  };

  std::map<std::string, ElevatorInfo> elevators;
  std::map<std::string, OrderInfo> orders;

  std::thread driver_thread;
  std::thread network_thread;

  const std::string backup_filename = "orders.txt";

  int current_id = 0;

  void choose_elevator(const std::string& id, int floor, ButtonType type);
  void remove_elevator(const std::string& ip);
  bool order_exists(int floor, int type);
  void backup_orders();
  bool restore_orders();
};
