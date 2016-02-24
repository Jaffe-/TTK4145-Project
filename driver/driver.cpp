#include "hw_interface/elev.h"
#include "driver.hpp"
#include "../logger/logger.hpp"
#include <chrono>
#include <iostream>

bool is_internal(Button button)
{
  return button == Button::INTERNAL_1 ||
    button == Button::INTERNAL_2 ||
    button == Button::INTERNAL_3 ||
    button == Button::INTERNAL_4;
}

unsigned int internal_button_floor(Button button)
{
  return static_cast<int>(button);
}

std::ostream& operator<<(std::ostream& s, const DriverEvent& event) {
  s << "{";
  switch (event.type) {
  case DriverEvent::BUTTON_PRESS:
    s << "BUTTON_PRESS";
    break;
  case DriverEvent::FLOOR_SIGNAL:
    s << "FLOOR_SIGNAL";
    break;
  default:
    s << "CORRUPT";
    break;
  }
  s << ", button=" << static_cast<int>(event.button);
  s << ", floor=" << event.floor << "}";
  return s;
}

Driver::Driver(bool use_simulator)
{
  std::string driver_string = use_simulator ? "simulated" : "hardware";
  int rv;
  if ((rv = elev_init(use_simulator ? ET_simulation : ET_comedi))) {
    LOG_DEBUG("Started driver (" << driver_string << ")");
  }
  else {
    LOG_DEBUG("Failed to start driver (" << driver_string << "), rv = " << rv);
    // Exception?
    return;
  }
  
  int current_floor = initialize_position();
  if (current_floor >= 0) {
    LOG_DEBUG("Elevator is now at floor " << current_floor);
  }
  else {
    LOG_DEBUG("Failed to position the elevator at a known floor.");
    // ERROR
  }
  fsm.set_floor(current_floor);

}


void Driver::poll(int& last, int new_value, int invalid_value, DriverEvent event)
{
   if (new_value != last){
    last = new_value;
    if(new_value != invalid_value){
      LOG_DEBUG("New event generated: " << event);
      fsm.notify(event);
      // Make event message :)
    }
  }
}
void Driver::event_generator()
{
  int floor_signal = elev_get_floor_sensor_signal();
  poll(last_floor_signal, floor_signal, -1, {DriverEvent::FLOOR_SIGNAL, NONE, floor_signal});

  for (int i = 0; i < FLOORS; i++) {
    for (int j = 0; j <= 2; j++ ) {
      if (i == 0 && j == 1 || i == 3 && j == 0) {
	continue;
      }
      
      int button_signal = elev_get_button_signal(static_cast<elev_button_type_t>(j), i);
      /*
      if (button_signal != 0) {
	LOG_DEBUG("button_signal = " << button_signal);
	LOG_DEBUG("last_button = " << last_button_signals[i][j]);
      }
      */
      poll(last_button_signals[i][j], button_signal, 0,
	   {DriverEvent::BUTTON_PRESS, button_list[i][j], -1});
    }
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
	LOG_DEBUG("Stopping motor");
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
    event_generator();
    fsm.run();
  }
  /*
  switch (state) {
  case FSMState::MOVING:
    if (elev_get_floor_sensor_signal() == floors[0]) {
      elev_set_motor_direction(DIRN_STOP);
      state = FSMState::STOPPED;
    }
  case FSMState::STOPPED:
    
  }
  */
}
