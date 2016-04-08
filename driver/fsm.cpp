#include "fsm.hpp"
#include "../util/logger.hpp"

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
 
