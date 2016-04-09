#include "fsm.hpp"

int SimulatedFSM::calculate(int floor, int type)
{
  insert_order(floor, type);
  int step = 0;
  while (state.current_floor != floor) {
    if (should_stop(state.current_floor))
      step++;
    if (state.direction == Direction::UP) {
      if (floors_above()) {
	state.current_floor++;
	step++;
      }
      else if (floors_below()) {
	state.direction = Direction::DOWN;
	state.current_floor--;
	step++;
      }
    }
    else if (state.direction == Direction::DOWN) {
      if (floors_below()) {
	state.current_floor--;
	step++;
      }
      else if (floors_above()) {
	state.direction = Direction::UP;
	state.current_floor++;
	step++;
      }
    }
  }
  return step;
}
