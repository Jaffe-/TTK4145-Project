#pragma once

#include <vector>
#include <chrono>
#include <iostream>
#include "fsm.hpp"

#define FLOORS 4

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

enum Button {
  INTERNAL_1, INTERNAL_2, INTERNAL_3, INTERNAL_4,
  EXTERNAL_1U,
  EXTERNAL_2D, EXTERNAL_2U,
  EXTERNAL_3D, EXTERNAL_3U,
  EXTERNAL_4D,
  NONE
};

const Button button_list[FLOORS][3] = {
  { EXTERNAL_1U, NONE, INTERNAL_1},
  { EXTERNAL_2U, EXTERNAL_2D, INTERNAL_2},
  { EXTERNAL_3U, EXTERNAL_3D, INTERNAL_3},
  { NONE, EXTERNAL_4D, INTERNAL_4}
};

bool is_internal(Button button);
unsigned int internal_button_floor(Button button);

struct DriverEvent {

  enum Type {
    BUTTON_PRESS, FLOOR_SIGNAL
  };

  Type type;
  Button button;
  int floor;
};

std::ostream& operator<<(std::ostream& s, const DriverEvent& event);
class Driver {
public:
  Driver(bool use_simulator);
  
  void update_floors(std::vector<unsigned int> new_floors);
  void run();
  
private:
  void notify_fsm(DriverEvent event);
  void insert_order(unsigned int floor);
  void poll(int& last, int new_value, int invalid_value, DriverEvent event);
  void event_generator();
  int initialize_position();

  FSM fsm;
  int last_floor_signal = -1;
  int last_button_signals[FLOORS][3] {};
};
