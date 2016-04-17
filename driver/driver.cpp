#include "hw_interface/elev.h"
#include "driver.hpp"
#include "../util/logger.hpp"
#include "../util/init_exception.hpp"
#include <chrono>
#include <iostream>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

Driver::Driver(EventQueue& logic_queue, bool use_simulator)
  : logic_queue(logic_queue),
    fsm(logic_queue)
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
    throw InitException();
  }

  fsm.set_floor(current_floor);
}

template <typename EventType>
void Driver::poll(int& last, int new_value, int invalid_value, const EventType& event)
{
  if (new_value != last){
    last = new_value;
    if (new_value != invalid_value){
      LOG_DEBUG("New event generated: " << event);
      fsm.notify(event);
      logic_queue.push(event);
    }
  }
}

void Driver::event_generator()
{
  /* Poll for floor signal */
  int floor_signal = elev_get_floor_sensor_signal();
  poll(last_floor_signal, floor_signal, -1, FloorSignalEvent(floor_signal));

  for (int floor = 0; floor < FLOORS; floor++) {
    for (auto button_type : {ButtonType::UP, ButtonType::DOWN}) {
      /* first floor doesn't have a down button and top floor doesn't have an up
	 button */
      if ((floor == 0 && button_type == ButtonType::DOWN)
	  || (floor == 3 && button_type == ButtonType::UP)) {
	continue;
      }
      int btn_index = static_cast<int>(button_type);
      int button_signal = elev_get_button_signal(static_cast<elev_button_type_t>(btn_index), floor);
      poll(last_button_signals[floor][btn_index], button_signal, 0,
	   ExternalButtonEvent(floor, button_type));
    }

    /* Internal buttons: */
    int button_signal = elev_get_button_signal(BUTTON_COMMAND, floor);
    poll(last_button_signals[floor][2], button_signal, 0,
	 InternalButtonEvent(floor));
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
    event_queue.handle_events(event_queue.acquire(), &fsm, events);
    fsm.run();
    event_generator();
  }
}

std::ostream& operator<<(std::ostream& s, const InternalButtonEvent& event) {
  return s << "{InternalButtonEvent floor=" << event.floor << "}";
}

std::ostream& operator<<(std::ostream& s, const ExternalButtonEvent& event) {
  return s << "{ExternalButtonEvent floor=" << event.floor
	   << " type=" << static_cast<int>(event.type) << "}";
}

std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event) {
  return s << "{FloorSignalEvent floor=" << event.floor << "}";
}

std::ostream& operator<<(std::ostream& s, const StateUpdateEvent& event) {
  return s << "{StateUpdateEvent cf=" << event.state.current_floor
    << " dir=" << int(event.state.direction);
}

std::ostream& operator<<(std::ostream& s, const FSMOrderCompleteEvent& event) {
  return s << "{FSMOrderCompleteEvent floor=" << event.floor << " type= " << event.type << "}";
}
