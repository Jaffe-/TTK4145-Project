#pragma once

#include <iostream>
#include "fsm.hpp"
#include "../util/event_queue.hpp"

class Driver {
public:
  Driver(EventQueue& logic_queue, bool use_simulator);
  void run();

  EventQueue& logic_queue;
  EventQueue event_queue;

private:
  void insert_order(unsigned int floor);
  template <typename EventType>
  void poll(int& last, int new_value, int invalid_value, EventType event);
  void event_generator();
  int initialize_position();

  FSM fsm;
  int last_floor_signal = -1;
  int last_button_signals[FLOORS][3] {};
};
