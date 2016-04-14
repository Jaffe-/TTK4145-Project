#pragma once

#include <ostream>
#include "../util/serialization.hpp"
#define FLOORS 4

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

enum StateID {
  MOVING, STOPPED
};

enum class Direction {
  UP, DOWN
};

enum class ButtonType {
  UP, DOWN, CMD
};

struct State {
  int current_floor = 0;
  Direction direction = Direction::UP;
  std::vector<std::vector<bool>> orders = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
  bool door_open = false;
  TimePoint door_opened_time;
  StateID state_id = STOPPED;
};

/* Events */
struct ExternalButtonEvent : public SerializableEvent {
  ExternalButtonEvent(int floor, ButtonType type, const std::string& id = "")
    : floor(floor),
      type(type),
      id(id)
  {
  }

  ExternalButtonEvent(const json_t& json) {
    floor = json["floor"];
    type = ButtonType(int(json["type"]));
    id = json["id"];
  }

  virtual json_t get_json() const override {
    return {{"floor", floor},
	    {"type", int(type)},
	    {"id", id}};
  }

  int floor;
  ButtonType type;
  std::string id;
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

struct FSMOrderCompleteEvent : public Event {
  FSMOrderCompleteEvent(int floor, int type) : floor(floor), type(type) {}

  int floor;
  int type;
};

/* Convenient overloads for writing events to log etc. */
std::ostream& operator<<(std::ostream& s, const InternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const ExternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event);
std::ostream& operator<<(std::ostream& s, const StateUpdateEvent& event);
std::ostream& operator<<(std::ostream& s, const FSMOrderCompleteEvent& event);
