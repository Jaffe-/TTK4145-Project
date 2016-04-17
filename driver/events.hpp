#pragma once

#include <ostream>
#include "fsm.hpp"

enum class ButtonType {
  UP, DOWN, CMD
};

/* Events */
struct ExternalButtonEvent : public Event {
  ExternalButtonEvent(int floor, ButtonType type)
    : floor(floor),
      type(type)
  {
  }

  int floor;
  ButtonType type;
};

struct InternalButtonEvent : public Event {
  InternalButtonEvent(int floor) : floor(floor) {};

  int floor;
};

struct FloorSignalEvent : public Event {
  FloorSignalEvent(int f) : floor(f) {};
  int floor;
};

struct OrderUpdateEvent : public Event {
  OrderUpdateEvent(int floor, int direction) : floor(floor), direction(direction) {};
  int floor;
  int direction;
};

struct StateUpdateEvent : public SerializableEvent {
  StateUpdateEvent(State s) : state(s) {};

  StateUpdateEvent(const json_t& json) {
    state.current_floor = json["current_floor"];
    state.direction = Direction(int(json["direction"]));
    state.orders = json["orders"].get<std::vector<std::vector<bool>>>();
    state.door_open = json["door_open"];
    state.state_id = StateID(int(json["state_id"]));
    state.error = json["error"];
  };
  State state;

  virtual json_t get_json() const override {
    return {{"current_floor", state.current_floor},
	    {"direction", int(state.direction)},
	    {"orders", state.orders},
	    {"door_open", state.door_open},
	    {"state_id", int(state.state_id)},
	    {"error", state.error}};
  }
};

struct FSMOrderCompleteEvent : public Event {
  FSMOrderCompleteEvent(int floor, int type) : floor(floor), type(type) {}

  int floor;
  int type;
};

struct ExternalLightOnEvent : public Event {
  ExternalLightOnEvent(int floor, ButtonType type) : floor(floor), type(type) {}

  int floor;
  ButtonType type;
};

struct ExternalLightOffEvent : public Event {
  ExternalLightOffEvent(int floor, ButtonType type) : floor(floor), type(type) {}

  int floor;
  ButtonType type;
};

/* Convenient overloads for writing events to log etc. */
std::ostream& operator<<(std::ostream& s, const InternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const ExternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event);
std::ostream& operator<<(std::ostream& s, const StateUpdateEvent& event);
std::ostream& operator<<(std::ostream& s, const FSMOrderCompleteEvent& event);
