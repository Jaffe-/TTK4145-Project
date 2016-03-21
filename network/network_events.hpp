#pragma once

#include <string>
#include "../util/event_queue.hpp"
#include "../driver/driver_events.hpp"

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
struct NetworkReceiveEvent : public Event {
  NetworkReceiveEvent(const std::string& ip, const Data& data)
    : ip(ip),
      data(data) {};

  std::string ip;
  Data data;
};

std::ostream& operator<<(std::ostream& os, const NewConnectionEvent& event);
std::ostream& operator<<(std::ostream& os, const LostNetworkEvent& event);
std::ostream& operator<<(std::ostream& os, const LostConnectionEvent&);

template <typename Data>
std::ostream& operator<<(std::ostream& os, const NetworkReceiveEvent<Data>& event)
{
  return os << "{NetworkReceiveEvent ip=" << event.ip
	    << " data=" << event.data << "}";
}
