#pragma once

#include <chrono>
#include "../util/message_queue.hpp"
#include "driver_events.hpp"
#define FLOORS 4

class ButtonPressEvent;
class FloorSignalEvent;
class OrderUpdate;

class FSM {
public:
  FSM();
  void run();
  void set_floor(int floor) { current_floor = floor; };

  MessageQueue message_queue;

private:
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

  enum State {
    MOVING, STOPPED
  };
  enum Direction {
    UP, DOWN
  };

  void clear_orders(int floor);
  void insert_order(int floor, int type);
  bool should_stop(int floor);
  void change_state(const State& new_state);
  bool floors_below();
  bool floors_above();
  void update_lights();
  void notify(const InternalButtonEvent& event);
  void notify(const FloorSignalEvent& event);
  void notify(const OrderUpdateEvent& event);
  
  State state;
  int current_floor;
  Direction direction;
  bool orders[FLOORS][3] {};
  bool door_open;
  const std::chrono::duration<double> door_time = std::chrono::seconds(3);
  TimePoint door_opened_time;

};
