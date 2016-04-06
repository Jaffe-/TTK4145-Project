#include "fsm.hpp"

int SimulatedFSM::calculate(Button button)
{
  insert_order(button_floor(button), button_type(button));
  int step = 0;
  while (state.current_floor != button_floor(button)) {
    if (should_stop(state.current_floor))
      step++;
    if (state.direction == UP) {
      if (floors_above()) {
	state.current_floor++;
	step++;
      }
      else if (floors_below()) {
	state.direction = DOWN;
	state.current_floor--;
	step++;
      }
    }
    else if (state.direction == DOWN) {
      if (floors_below()) {
	state.current_floor--;
	step++;
      }
      else if (floors_above()) {
	state.direction = UP;
	state.current_floor++;
	step++;
      }
    }
  }
  return step;
}
