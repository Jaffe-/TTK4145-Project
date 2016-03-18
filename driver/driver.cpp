#include "hw_interface/elev.h"
#include "driver.hpp"
#include "../util/logger.hpp"
#include <chrono>
#include <iostream>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

Driver::Driver(EventQueue& logic_queue, bool use_simulator)
  : logic_queue(logic_queue),
    event_queue(fsm.event_queue)
{
  std::string driver_string = use_simulator ? "simulated" : "hardware";
  elev_init(use_simulator ? ET_Simulation : ET_Comedi);

  LOG_INFO("Driver started (" << driver_string << ")");

  int current_floor = initialize_position();
  if (current_floor >= 0) {
    LOG_INFO("Elevator is now at floor " << current_floor);
  }
  else {
    LOG_ERROR("Failed to position the elevator at a known floor!");
    // ERROR
  }

  fsm.set_floor(current_floor);
}

template <typename EventType>
void Driver::poll(int& last, int new_value, int invalid_value, EventType event)
{
   if (new_value != last){
    last = new_value;
    if(new_value != invalid_value){
      LOG_DEBUG("New event generated: " << event);
      fsm.event_queue.push(event);
      logic_queue.push(event);
    }
  }
}

void Driver::event_generator()
{
  /* Poll for floor signal */
  int floor_signal = elev_get_floor_sensor_signal();
  poll(last_floor_signal, floor_signal, -1, FloorSignalEvent(floor_signal));

  for (int i = 0; i < FLOORS; i++) {
    /* Go through external up and external down (0 and 1) buttons */
    for (int j = 0; j <= 1; j++ ) {
      if ((i == 0 && j == 1) || (i == 3 && j == 0)) {
	/* first floor doesn't have a down button and top floor doesn't have an up
	   button */
	continue;
      }

      int button_signal = elev_get_button_signal(static_cast<elev_button_type_t>(j), i);
      poll(last_button_signals[i][j], button_signal, 0,
	   ExternalButtonEvent(button_list[i][j]));
    }

    /* Internal buttons: */
    int button_signal = elev_get_button_signal(BUTTON_COMMAND, i);
    poll(last_button_signals[i][2], button_signal, 0,
	 InternalButtonEvent(button_list[i][2]));
  }
}

int Driver::initialize_position()
{
  const std::chrono::duration<double> wait_time = std::chrono::seconds(4);
  for (elev_motor_direction_t dir : {DIRN_UP, DIRN_DOWN}) {
    LOG_DEBUG("Trying to go " << (dir == DIRN_UP ? "up" : "down"));
    elev_set_motor_direction(dir);
    TimePoint start = std::chrono::system_clock::now();
    while (std::chrono::system_clock::now() - start < wait_time) {
      int floor_signal = elev_get_floor_sensor_signal();
      if (floor_signal != -1) {
	elev_set_motor_direction(DIRN_STOP);
	return floor_signal;
      }
    }
  }
  return -1;
}

void Driver::run()
{
  while (1) {
    fsm.run();
    event_generator();
  }
}

std::ostream& operator<<(std::ostream& s, const InternalButtonEvent& event) {
  s << "{InternalButtonEvent button=" << static_cast<int>(event.button) << "}";
  return s;
}

std::ostream& operator<<(std::ostream& s, const ExternalButtonEvent& event) {
  s << "{ExternalButtonEvent button=" << static_cast<int>(event.button) << "}";
  return s;
}

std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event) {
  s << "{FloorSignalEvent floor=" << event.floor << "}";
  return s;
}
