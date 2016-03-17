#pragma once

#include <ostream>

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

/* Events */
struct ExternalButtonEvent : public Message {
  ExternalButtonEvent(Button b) : button(b) {};
  Button button;
};

struct InternalButtonEvent : public Message {
  InternalButtonEvent(Button b) : button(b) {};
  Button button;
};

struct FloorSignalEvent : public Message {
  FloorSignalEvent(int f) : floor(f) {};
  int floor;
};

struct OrderUpdateEvent : public Message {
  OrderUpdateEvent(int floor, int direction) : floor(floor), direction(direction) {};
  int floor;
  int direction;
};

/* Convenient overloads for writing events to log etc. */
std::ostream& operator<<(std::ostream& s, const InternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const ExternalButtonEvent& event);
std::ostream& operator<<(std::ostream& s, const FloorSignalEvent& event);

int button_floor(Button button);
int button_type(Button button);
