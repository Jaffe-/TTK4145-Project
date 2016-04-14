#pragma once

#include <string>
#include "../util/event_queue.hpp"

struct NewConnectionEvent : public Event {
  NewConnectionEvent(const std::string& ip) : ip(ip) {};
  std::string ip;
};

struct LostConnectionEvent : public Event {
  LostConnectionEvent(const std::string& ip) : ip(ip) {};
  std::string ip;
};

struct LostNetworkEvent : public Event {
  LostNetworkEvent() {};
};

template <typename Data>
struct NetworkMessageEvent : public SerializableEvent {
  NetworkMessageEvent(const std::string& ip, const Data& data)
    : ip(ip),
      data(data) {
    assert(data.serializable() && "Data needs to be serializable");
  };

  virtual json_t get_json() const override {
    return {{"ip", ip}, {"data", data.get_json() }};
  }

  std::string ip;
  Data data;
};

std::ostream& operator<<(std::ostream& os, const NewConnectionEvent& event);
std::ostream& operator<<(std::ostream& os, const LostNetworkEvent& event);
std::ostream& operator<<(std::ostream& os, const LostConnectionEvent&);

template <typename Data>
std::ostream& operator<<(std::ostream& os, const NetworkMessageEvent<Data>& event)
{
  return os << "{NetworkMessageEvent ip=" << event.ip
	    << " data=" << event.data << "}";
}
