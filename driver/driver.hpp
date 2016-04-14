#pragma once

#include <iostream>
#include "physicalfsm.hpp"
#include "../util/event_queue.hpp"

class Driver {
public:
  Driver(EventQueue& logic_queue, bool use_simulator);
  void run();

  EventQueue& logic_queue;
  EventQueue event_queue;

private:

  /* List of events that this module handles */
  EventList<OrderUpdateEvent> events;

  template <typename EventType>
  void poll(int& last, int new_value, int invalid_value, EventType event);

  void event_generator();
  int initialize_position();

  PhysicalFSM fsm;
  int last_floor_signal = -1;
  int last_button_signals[FLOORS][3] {};
};
