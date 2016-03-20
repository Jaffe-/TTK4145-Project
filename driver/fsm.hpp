#pragma once

#include <chrono>
#include "../util/event_queue.hpp"
#include "driver_events.hpp"
#define FLOORS 4

class ButtonPressEvent;
class FloorSignalEvent;
class OrderUpdate;

class FSM {
public:
  FSM(EventQueue& logic_queue);
  void run();
  void set_floor(int floor) { state.current_floor = floor; };

  EventQueue event_queue;

private:
  void clear_orders(int floor);
  void insert_order(int floor, int type);
  bool should_stop(int floor);
  void change_state(const StateID& new_state);
  bool floors_below();
  bool floors_above();
  void update_lights();
  void notify(const InternalButtonEvent& event);
  void notify(const FloorSignalEvent& event);
  void notify(const OrderUpdateEvent& event);
  void send_state();
  
  State state;

  const std::chrono::duration<double> door_time = std::chrono::seconds(3);

  EventQueue& logic_queue;
};
