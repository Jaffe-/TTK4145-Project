#pragma once
#include "json.hpp"
#include <typeinfo>
#include "logger.hpp"

using json_t = nlohmann::json;

/* Basic event class */
class Event {
public:
  virtual bool serializable() const {
    return false;
  };
};

/* Event that can be serialized */
class SerializableEvent : public Event {
public:
  virtual bool serializable() const override {
    return true;
  }

  // By default, return empty json
  virtual json_t get_json() const {
    return {};
  }

  std::string serialize() const {
    return get_json().dump();
  }
};

/* Event lists used to list event class types for use in the event queues
   and in the network receiver */
template <typename... EventTypes>
struct EventList {};
