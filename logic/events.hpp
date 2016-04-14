#pragma once

#include "../util/event.hpp"
#include <vector>

struct StateUpdateReqEvent : public SerializableEvent {

  StateUpdateReqEvent() {};
  StateUpdateReqEvent(const json_t&) {};
};

struct OrderCompleteEvent : public SerializableEvent {
  OrderCompleteEvent(const std::string& id) : id(id) {}
  OrderCompleteEvent(const json_t& json) {
    id = json["id"];
  }

  virtual json_t get_json() const override {
    return {{"id", id}};
  }

  std::string id;
};
