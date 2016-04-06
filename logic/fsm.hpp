#pragma once

#include "../driver/fsm.hpp"

class SimulatedFSM : public FSM {
public:
  SimulatedFSM(State state_) : FSM() { state = state_; };
  int calculate(Button button);
};
