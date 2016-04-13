#include "fsm.hpp"
#include "../util/logger.hpp"

bool FSM::at_floor(int floor)
{
  return floor == state.current_floor && state.state_id == STOPPED;
}

bool FSM::should_stop(int floor)
{
  return
    state.orders[floor][2]
    || (state.direction == Direction::UP && (state.orders[floor][0] || (state.orders[floor][1] && !floors_above())))
    || (state.direction == Direction::DOWN && (state.orders[floor][1] || (state.orders[floor][0] && !floors_below())));
}

void FSM::clear_orders(int floor)
{
  for (int i = 0; i <= 2; i++)
    state.orders[floor][i] = 0;
}

void FSM::insert_order(int floor, int type)
{
  if (!at_floor(floor)) {
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
