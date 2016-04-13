#include "simulatedfsm.hpp"
#include "../util/logger.hpp"

int SimulatedFSM::calculate(int floor, int type)
{
  insert_order(floor, type);
  int step = 0;
  while (!at_floor(floor))  {
		LOG_DEBUG("dir=" << (int)state.direction << " floor=" << state.current_floor
							<< "state=" << (int)state.state_id);
		if (state.state_id == MOVING) {
			if (should_stop(state.current_floor)) {
				state.state_id = STOPPED;
				step++;
				continue;
			}
		}
		else {
			state.state_id = MOVING;
		}

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
