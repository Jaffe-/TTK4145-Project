#pragma once

#include "../util/event.hpp"
#include <vector>

struct StateUpdateReqEvent : public SerializableEvent {

  StateUpdateReqEvent() {};
  StateUpdateReqEvent(const json_t&) {};
};
