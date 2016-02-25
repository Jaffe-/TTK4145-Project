#include "hw_interface/elev.h"
#include "fsm.hpp"
#include "driver.hpp"
#include "../util/logger.hpp"

FSM::FSM()
{
  state = STOPPED;
  door_open = false;
  direction = UP;
}

bool FSM::should_stop(int floor)
{
  return
    orders[floor][2] ||
    (direction == UP && orders[floor][0]) ||
    (direction == DOWN && orders[floor][1]);
}

void FSM::clear_orders(int floor)
{
  for (int i = 0; i <= 2; i++)
    orders[floor][i] = 0;
}

void FSM::insert_order(int floor, int type)
{
  LOG_DEBUG("New order: go to floor " << floor << ", type=" << type);
  orders[floor][type] = true;
}

void FSM::change_state(const State& new_state)
{
  if (new_state == STOPPED) {
    LOG_DEBUG("Changed state to STOPPED");
    elev_set_motor_direction(DIRN_STOP);
    door_opened_time = std::chrono::system_clock::now();
    door_open = true;
    clear_orders(current_floor);
  }
  else if (new_state == MOVING) {
    LOG_DEBUG("Changed state to MOVING");
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

void FSM::notify(const DriverEvent& event)
{
  if (event.type == DriverEvent::BUTTON_PRESS) {
    if (is_internal(event.button)) {
      insert_order(internal_button_floor(event.button), 2);
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
    if (orders[floor][0] || orders[floor][2]) {
      return true;
    }
  }
  return false;
}

bool FSM::floors_below()
{
  for (int floor = current_floor - 1; floor >= 0; floor--) {
    if (orders[floor][1] || orders[floor][2]) {
      return true;
    }
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
      if (direction == UP) {
	if (floors_above()) {
	  change_state(MOVING);
	}
	else if (floors_below()) {
	  direction = DOWN;
	  change_state(MOVING);
	}
      }
      else if (direction == DOWN) {
	if (floors_below()) {
	  change_state(MOVING);
	}
	else if (floors_above()) {
	  direction = UP;
	  change_state(MOVING);
	}
      }
    }
  }
}
