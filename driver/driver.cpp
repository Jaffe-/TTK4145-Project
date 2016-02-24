#include "hw_interface/elev.h"
#include "driver.hpp"
#include "../logger/logger.hpp"
#include <chrono>

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

Driver::Driver(bool use_simulator)
{
  std::string driver_string = use_simulator ? "simulated" : "hardware";
  if (elev_init(use_simulator ? ET_simulation : ET_comedi)) {
    LOG_DEBUG("Started driver (" << driver_string << ")");
  }
  else {
    LOG_DEBUG("Failed to start driver (" << driver_string << ")");
    // Exception?
  }
}

bool FSM::should_stop(int floor)
{
  return
    orders[floor][2] ||
    direction == UP && orders[floor][0] || 
    direction == DOWN && orders[floor][1];
}

void FSM::change_state(State new_state)
{
  if (new_state == STOPPED) {
    elev_set_motor_direction(DIRN_STOP);
    door_opened_time = std::chrono::system_clock::now();
    door_open = true;
  }
  else if (new_state == MOVING) {
    if (direction == UP)
      elev_set_motor_direction(DIRN_UP);
    else
      elev_set_motor_direction(DIRN_DOWN);
  }
  state = new_state;
}

void FSM::update_lights()
{
  elev_set_floor_indicator(current_floor);
  elev_set_door_open_lamp(door_open ? 1 : 0);
  for (int floor = 0; floor < FLOORS; floor++) {
    elev_set_button_lamp(BUTTON_COMMAND, floor, orders[floor][2] ? 1 : 0);
  }
}

void FSM::notify(DriverEvent event)
{
  if (event.type == DriverEvent::BUTTON_PRESS) {
    if (is_internal(event.button)) {
      orders[internal_button_floor(event.button)][2] = 1;
    }
  }
  else if (event.type == DriverEvent::FLOOR_SIGNAL) {
    current_floor = event.floor;
    if (should_stop(event.floor)) {
      change_state(STOPPED);
    }
  }
  update_lights();
}

bool FSM::floors_above()
{
  for (int floor = current_floor + 1; floor < FLOORS; floor++) {
    if (orders[floor][0] || orders[floor][2])
      return true;
  }
  return false;
}

bool FSM::floors_below()
{
  for (int floor = current_floor - 1; floor >= 0; floor--) {
    if (orders[floor][1] || orders[floor][2])
      return true;
  }
  return false;
}

void FSM::run()
{
  if (state == STOPPED) {
    if (door_open) {
      if (std::chrono::system_clock::now() - door_opened_time > door_time) {
	door_open = false;
      }
    }
    else {
      if (direction == UP && !floors_above())
	direction = DOWN;
      else if (direction == DOWN && !floors_below())
	direction = UP;
      
      change_state(MOVING);
    }
  }
}

void Driver::run()
{
  
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
