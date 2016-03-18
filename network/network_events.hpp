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

std::ostream& operator<<(std::ostream& os, const NewConnectionEvent& event);
std::ostream& operator<<(std::ostream& os, const LostNetworkEvent& event);
std::ostream& operator<<(std::ostream& os, const LostConnectionEvent& event);
