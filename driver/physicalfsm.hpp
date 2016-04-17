#pragma once
#include "fsm.hpp"
#include "events.hpp"

class PhysicalFSM : public FSM {
public:
  PhysicalFSM(EventQueue& logic_queue);
  void notify(const InternalButtonEvent& event);
  void notify(const FloorSignalEvent& event);
  void notify(const OrderUpdateEvent& event);
  void notify(const ExternalButtonEvent& event);
  void notify(const ExternalLightOnEvent& event);
  void notify(const ExternalLightOffEvent& event);

  void run();
  void set_floor(int floor) { state.current_floor = floor; };

private:
  void change_state(const StateID& new_state);
  void update_lights();
  void send_state();
  void open_door();

  EventQueue& logic_queue;
  TimePoint depart_time;
  const std::chrono::duration<double> move_timeout = std::chrono::seconds(4);
};
