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
