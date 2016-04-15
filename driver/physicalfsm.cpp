#include "physicalfsm.hpp"
#include "../util/logger.hpp"
#include "hw_interface/elev.h"

PhysicalFSM::PhysicalFSM(EventQueue& logic_queue) :
  FSM(),
  logic_queue(logic_queue)
{
}

void PhysicalFSM::open_door()
{
  state.door_opened_time = std::chrono::system_clock::now();
  state.door_open = true;
}

void PhysicalFSM::change_state(const StateID& new_state)
{
  if (new_state == STOPPED) {
    LOG_DEBUG("Changed state to STOPPED");
    elev_set_motor_direction(DIRN_STOP);
    state.door_opened_time = std::chrono::system_clock::now();
    state.door_open = true;
    for (int i = 0; i < 2; i++) {
      if (state.orders[state.current_floor][i]) {
	logic_queue.push(FSMOrderCompleteEvent(state.current_floor, i));
      }
    }
    clear_orders(state.current_floor);
  }
  else if (new_state == MOVING) {
    LOG_DEBUG("Changed state to MOVING");
    if (state.direction == Direction::UP)
      elev_set_motor_direction(DIRN_UP);
    else
      elev_set_motor_direction(DIRN_DOWN);
    depart_time = std::chrono::system_clock::now();
  }
  state.state_id = new_state;
}

void PhysicalFSM::update_lights()
{
  elev_set_floor_indicator(state.current_floor);
  elev_set_door_open_lamp(state.door_open ? 1 : 0);
  for (int floor = 0; floor < FLOORS; floor++) {
    elev_set_button_lamp(BUTTON_COMMAND, floor, state.orders[floor][2] ? 1 : 0);
  }
}

void PhysicalFSM::notify(const ExternalButtonEvent&)
{
}

void PhysicalFSM::notify(const InternalButtonEvent& event)
{
  insert_order(event.floor, 2);
  update_lights();
  send_state();
}

void PhysicalFSM::notify(const FloorSignalEvent& event)
{
  state.current_floor = event.floor;
  if (should_stop(event.floor)) {
    change_state(STOPPED);
  }
  state.error = false;
  update_lights();
  send_state();
  depart_time = std::chrono::system_clock::now();
}

void PhysicalFSM::notify(const OrderUpdateEvent& event)
{
  LOG_DEBUG("New order: go to floor " << event.floor
	    << ", type=" << event.direction);
  insert_order(event.floor, event.direction);
  if (at_floor(event.floor)) {
    if (!state.door_open) {
      open_door();
    }
  }
  update_lights();
  send_state();
}

void PhysicalFSM::run()
{
  if (state.state_id == STOPPED) {
    if (state.door_open) {
      if (std::chrono::system_clock::now() - state.door_opened_time > door_time) {
	state.door_open = false;
	update_lights();
	send_state();
      }
    }
    else {
      if (state.direction == Direction::UP) {
	if (floors_above()) {
	  change_state(MOVING);
	  send_state();
	}
	else if (floors_below()) {
	  state.direction = Direction::DOWN;
	  change_state(MOVING);
	  send_state();
	}
      }
      else if (state.direction == Direction::DOWN) {
	if (floors_below()) {
	  change_state(MOVING);
	  send_state();
	}
	else if (floors_above()) {
	  state.direction = Direction::UP;
	  change_state(MOVING);
	  send_state();
	}
      }
    }
  }
  else {
    if (!state.error && std::chrono::system_clock::now() - depart_time > move_timeout) {
      state.error = true;
      send_state();
    }
  }
}

void PhysicalFSM::send_state()
{
  logic_queue.push(StateUpdateEvent(state));
}
