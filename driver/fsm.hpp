#pragma once

#include <chrono>

#define FLOORS 4

class DriverEvent;

class FSM {
public:
  FSM();
  void notify(const DriverEvent& event);
  void run();
  void set_floor(int floor) { current_floor = floor; };

private:
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

  enum State {
    MOVING, STOPPED
  };
  enum Direction {
    UP, DOWN
  };

  bool should_stop(int floor);
  void change_state(const State& new_state);
  bool floors_below();
  bool floors_above();
  void update_lights();
  
  State state;
  int current_floor;
  Direction direction;
  bool orders[FLOORS][3] {};
  bool door_open;
  const std::chrono::duration<double> door_time = std::chrono::seconds(3);
  TimePoint door_opened_time;
};
