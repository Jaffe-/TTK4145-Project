#pragma once

#include <ostream>
#include "../util/serialization.hpp"
#define FLOORS 4

enum Button {
  INTERNAL_1, INTERNAL_2, INTERNAL_3, INTERNAL_4,
  EXTERNAL_1U,
  EXTERNAL_2D, EXTERNAL_2U,
  EXTERNAL_3D, EXTERNAL_3U,
  EXTERNAL_4D,
  NONE
};

/* Maps button matrix indices to Button enum values */
const Button button_list[FLOORS][3] = {
  { EXTERNAL_1U, NONE, INTERNAL_1},
  { EXTERNAL_2U, EXTERNAL_2D, INTERNAL_2},
  { EXTERNAL_3U, EXTERNAL_3D, INTERNAL_3},
  { NONE, EXTERNAL_4D, INTERNAL_4}
};

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

enum StateID {
  MOVING, STOPPED
};
enum Direction {
  UP, DOWN
};

struct State {
  int current_floor = 0;
  Direction direction = UP;
  std::vector<std::vector<bool>> orders = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
  bool door_open = false;
  TimePoint door_opened_time;
  StateID state_id = STOPPED;
};

/* Events */
struct ExternalButtonEvent : public SerializableEvent {
  ExternalButtonEvent(Button b) : button(b) {};
  ExternalButtonEvent(const json_t& json) {
    button = Button(int(json["button"]));
  }

  virtual json_t get_json() const override {
    return {{"button", int(button)}};
  }

  Button button;
};

struct InternalButtonEvent : public Event {
  InternalButtonEvent(Button b) : button(b) {};
  Button button;
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
  };
  State state;

  virtual json_t get_json() const override {
    return {{"current_floor", state.current_floor},
	    {"direction", int(state.direction)},
	    {"orders", state.orders},
	    {"door_open", state.door_open},
	    {"state_id", int(state.state_id)}};
  }
};

/* Convenient overloads for writing events to log etc. */
std::ostream& operator<<(std::ostream& s, const InternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const ExternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event);
std::ostream& operator<<(std::ostream& s, const StateUpdateEvent& event);

int button_floor(Button button);
int button_type(Button button);
