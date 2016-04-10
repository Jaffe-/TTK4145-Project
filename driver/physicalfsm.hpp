#pragma once
#include "fsm.hpp"

class PhysicalFSM : public FSM {
public:
  PhysicalFSM(EventQueue& logic_queue);
  void notify(const InternalButtonEvent& event);
  void notify(const FloorSignalEvent& event);
  void notify(const OrderUpdateEvent& event);
  void notify(const ExternalButtonEvent&);
  void run();
  void set_floor(int floor) { state.current_floor = floor; };

private:
  void change_state(const StateID& new_state);
  void update_lights();
  void send_state();

  EventQueue& logic_queue;
};