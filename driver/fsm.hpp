#pragma once

#include <chrono>
#include "../util/event_queue.hpp"
//#include "events.hpp"
#define FLOORS 4

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

enum StateID {
  MOVING, STOPPED
};

enum class Direction {
  UP, DOWN
};

struct State {
  int current_floor = 0;
  Direction direction = Direction::UP;
  std::vector<std::vector<bool>> orders = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
  bool door_open = false;
  TimePoint door_opened_time;
  StateID state_id = STOPPED;
  bool error = false;
};

class FSM {
protected:
  bool at_floor(int floor);
  void clear_orders(int floor);
  void insert_order(int floor, int type);
  bool should_stop(int floor);
  bool floors_below();
  bool floors_above();

  State state;

  const std::chrono::duration<double> door_time = std::chrono::seconds(3);
};
