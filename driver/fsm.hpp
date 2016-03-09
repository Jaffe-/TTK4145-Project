#pragma once

#include <chrono>
#include "../util/message_queue.hpp"
#include <thread>

#define FLOORS 4

class ButtonPressEvent;
class FloorSignalEvent;
class OrderUpdate;

class FSM {
public:
  FSM(MessageQueue& msg_queue)
    : message_queue(msg_queue), state(STOPPED), direction(UP), door_open(false) {};
  void run();
  void set_floor(int floor) { current_floor = floor; };

  MessageQueue& message_queue;

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
  void notify(const ButtonPressEvent& event);
  void notify(const FloorSignalEvent& event);
  void order_update(const OrderUpdate& order_update);
  
  State state;
  int current_floor;
  Direction direction;
  bool orders[FLOORS][3] {};
  bool door_open;
  const std::chrono::duration<double> door_time = std::chrono::seconds(3);
  TimePoint door_opened_time;

};
