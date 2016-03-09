#pragma once

#include <vector>
#include <chrono>
#include <iostream>
#include "fsm.hpp"
#include "../util/message_queue.hpp"

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

struct ButtonPressEvent : public Message {
  Button button;
};

struct FloorSignalEvent : public Message {
  int floor;
};

struct OrderUpdate : public Message {

};

std::ostream& operator<<(std::ostream& s, const ButtonPressEvent& event);
std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event);

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
  std::thread fsm_thread;
  int last_floor_signal = -1;
  int last_button_signals[FLOORS][3] {};
};
