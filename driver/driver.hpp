#pragma once

#include <iostream>
#include "fsm.hpp"
#include "../util/message_queue.hpp"

class Driver {
public:
  Driver(bool use_simulator);
  void run();

  MessageQueue message_queue;
  
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
