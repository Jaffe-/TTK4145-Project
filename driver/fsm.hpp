#pragma once

#include <chrono>
#include "../util/event_queue.hpp"
#include "driver_events.hpp"
#define FLOORS 4

class ButtonPressEvent;
class FloorSignalEvent;
class OrderUpdate;

class FSM {
protected:
  void clear_orders(int floor);
  void insert_order(int floor, int type);
  bool should_stop(int floor);
  bool floors_below();
  bool floors_above();

  State state;

  const std::chrono::duration<double> door_time = std::chrono::seconds(3);

public:
  //  FSM();
};

class PhysicalFSM : public FSM {
public:
  PhysicalFSM(EventQueue& logic_queue);
  void notify(const InternalButtonEvent& event);
  void notify(const FloorSignalEvent& event);
  void notify(const OrderUpdateEvent& event);
  void notify(const ExternalButtonEvent&);
  void run();
  void set_floor(int floor) { state.current_floor = floor; };

private:
  void change_state(const StateID& new_state);
  void update_lights();
  void send_state();

  EventQueue& logic_queue;
};
