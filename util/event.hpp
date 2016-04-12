#pragma once
#include "serialization.hpp"
#include <typeinfo>

/* Basic event class */
class Event {
public:
  virtual bool serializable() const {
    return false;
  };

  /* Used to cast an event to its concrete derived type */
  template <typename T>
  operator const T&() const {
    if (typeid(T) == typeid(*this))
      return static_cast<const T&>(*this);
    else throw std::bad_cast();
  }

  /* Used to cast itself to Serializable. Must be dynamic cast since
     Event doesn't itself derive from Serializable. */
  operator const Serializable&() const {
    return dynamic_cast<const Serializable&>(*this);
  };
};

/* Event that can be serialized */
class SerializableEvent : public Event,
			  public Serializable {
public:
  virtual bool serializable() const override {
    return true;
  }
};

/* Event list

   This is used to list events and is used by event_queue.listen() and by
   serialize_event().
*/
template <typename... EventTypes>
struct EventList {};
