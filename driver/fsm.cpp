#include "hw_interface/elev.h"
#include "fsm.hpp"
#include "../util/logger.hpp"

FSM::FSM(EventQueue& logic_queue) : logic_queue(logic_queue)
{
}

bool FSM::should_stop(int floor)
{
  return
    state.orders[floor][2]
    || (state.direction == UP && (state.orders[floor][0] || (state.orders[floor][1] && !floors_above())))
    || (state.direction == DOWN && (state.orders[floor][1] || (state.orders[floor][0] && !floors_below())));
}

void FSM::clear_orders(int floor)
{
  for (int i = 0; i <= 2; i++)
    state.orders[floor][i] = 0;
}

void FSM::insert_order(int floor, int type)
{
  if (floor != state.current_floor) {
    LOG_DEBUG("New order: go to floor " << floor << ", type=" << type);
    state.orders[floor][type] = true;
  }
}

void FSM::change_state(const StateID& new_state)
{
  if (new_state == STOPPED) {
    LOG_DEBUG("Changed state to STOPPED");
    elev_set_motor_direction(DIRN_STOP);
    state.door_opened_time = std::chrono::system_clock::now();
    state.door_open = true;
    clear_orders(state.current_floor);
  }
  else if (new_state == MOVING) {
    LOG_DEBUG("Changed state to MOVING");
    if (state.direction == UP)
      elev_set_motor_direction(DIRN_UP);
    else
      elev_set_motor_direction(DIRN_DOWN);
  }
  state.state_id = new_state;
}

void FSM::update_lights()
{
  elev_set_floor_indicator(state.current_floor);
  elev_set_door_open_lamp(state.door_open ? 1 : 0);
  for (int floor = 0; floor < FLOORS; floor++) {
    elev_set_button_lamp(BUTTON_COMMAND, floor, state.orders[floor][2] ? 1 : 0);
  }
}

void FSM::notify(const ExternalButtonEvent&)
{
}

void FSM::notify(const InternalButtonEvent& event)
{
  insert_order(button_floor(event.button), 2);
  update_lights();
  send_state();
}

void FSM::notify(const FloorSignalEvent& event)
{
  state.current_floor = event.floor;
  if (should_stop(event.floor)) {
    change_state(STOPPED);
  }
  update_lights();
  send_state();
}

void FSM::notify(const OrderUpdateEvent& event)
{
  insert_order(event.floor, event.direction);
  update_lights();
  send_state();
}

bool FSM::floors_above()
{
  for (int floor = state.current_floor + 1; floor < FLOORS; floor++) {
    if (state.orders[floor][0] || state.orders[floor][1] || state.orders[floor][2]) {
      return true;
    }
  }
  return false;
}

bool FSM::floors_below()
{
  for (int floor = state.current_floor - 1; floor >= 0; floor--) {
    if (state.orders[floor][0] || state.orders[floor][1] || state.orders[floor][2]) {
      return true;
    }
  }
  return false;
}

void FSM::run()
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
      if (state.direction == UP) {
	if (floors_above()) {
	  change_state(MOVING);
	  send_state();
	}
	else if (floors_below()) {
	  state.direction = DOWN;
	  change_state(MOVING);
	  send_state();
	}
      }
      else if (state.direction == DOWN) {
	if (floors_below()) {
	  change_state(MOVING);
	  send_state();
	}
	else if (floors_above()) {
	  state.direction = UP;
	  change_state(MOVING);
	  send_state();
	}
      }
    }
  }
}

void FSM::send_state()
{
  logic_queue.push(StateUpdateEvent(state));
}

bool is_internal(Button button)
{
  return button == Button::INTERNAL_1 ||
    button == Button::INTERNAL_2 ||
    button == Button::INTERNAL_3 ||
    button == Button::INTERNAL_4;
}

int button_floor(Button button) {
  switch (button) {
  case INTERNAL_1:
  case EXTERNAL_1U:
    return 0;
  case INTERNAL_2:
  case EXTERNAL_2D:
  case EXTERNAL_2U:
    return 1;
  case INTERNAL_3:
  case EXTERNAL_3D:
  case EXTERNAL_3U:
    return 2;
  case INTERNAL_4:
  case EXTERNAL_4D:
    return 3;
  case NONE:
  default:
    return -1;
  }
}

int button_type(Button button)
{
  switch (button) {
  case EXTERNAL_1U:
  case EXTERNAL_2U:
  case EXTERNAL_3U:
    return 0;
  case EXTERNAL_2D:
  case EXTERNAL_3D:
  case EXTERNAL_4D:
    return 1;
  case INTERNAL_1:
  case INTERNAL_2:
  case INTERNAL_3:
  case INTERNAL_4:
    return 2;
  case NONE:
  default:
    return -1;
  }
}
 
