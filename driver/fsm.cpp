#include "hw_interface/elev.h"
#include "fsm.hpp"
#include "../util/logger.hpp"

FSM::FSM()
  : state(STOPPED),
    direction(UP),
    door_open(false)
{
  event_queue.add_handler<OrderUpdateEvent>(this, &FSM::notify);
  event_queue.add_handler<InternalButtonEvent>(this, &FSM::notify);
  event_queue.add_handler<FloorSignalEvent>(this, &FSM::notify);
}

bool FSM::should_stop(int floor)
{
  return
    orders[floor][2]
    || (direction == UP && (orders[floor][0] || (orders[floor][1] && !floors_above())))
    || (direction == DOWN && (orders[floor][1] || (orders[floor][0] && !floors_below())));
}

void FSM::clear_orders(int floor)
{
  for (int i = 0; i <= 2; i++)
    orders[floor][i] = 0;
}

void FSM::insert_order(int floor, int type)
{
  if (floor != current_floor) {
    LOG_DEBUG("New order: go to floor " << floor << ", type=" << type);
    orders[floor][type] = true;
  }
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

void FSM::notify(const InternalButtonEvent& event)
{
  insert_order(button_floor(event.button), 2);
  update_lights();
}

void FSM::notify(const FloorSignalEvent& event)
{
  current_floor = event.floor;
  if (should_stop(event.floor)) {
    change_state(STOPPED);
  }
  update_lights();
}

void FSM::notify(const OrderUpdateEvent& event)
{
  insert_order(event.floor, event.direction);
  update_lights();
}

bool FSM::floors_above()
{
  for (int floor = current_floor + 1; floor < FLOORS; floor++) {
    if (orders[floor][0] || orders[floor][1] || orders[floor][2]) {
      return true;
    }
  }
  return false;
}

bool FSM::floors_below()
{
  for (int floor = current_floor - 1; floor >= 0; floor--) {
    if (orders[floor][0] || orders[floor][1] || orders[floor][2]) {
      return true;
    }
  }
  return false;
}

void FSM::run()
{
  event_queue.handle_events(event_queue.acquire());

  if (state == STOPPED) {
    if (door_open) {
      if (std::chrono::system_clock::now() - door_opened_time > door_time) {
	door_open = false;
	update_lights();
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
 
