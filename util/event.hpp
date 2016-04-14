#pragma once
#include "serialization.hpp"
#include <typeinfo>
#include "logger.hpp"

/* Basic event class */
class Event {
public:
  virtual bool serializable() const {
    return false;
  };
};

/* Event that can be serialized */
class SerializableEvent : public Event,
			  public Serializable {
public:
  virtual bool serializable() const override {
    return true;
  }

  // By default, return empty json
  virtual json_t get_json() const override {
    return {};
  }
};

/* Event list

   This is used to list events and is used by event_queue.listen() and by
   serialize_event().
*/
template <typename... EventTypes>
struct EventList {};
