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

struct NetworkReceiveStateEvent : public Event {
  NetworkReceiveStateEvent(const std::string& ip, const StateUpdateEvent& update_event)
    : ip(ip),
      update_event(update_event) {};
  std::string ip;
  StateUpdateEvent update_event;
};

struct NetworkReceiveButtonEvent : public Event {
  NetworkReceiveButtonEvent(const std::string& ip, const ExternalButtonEvent& button_event)
    : ip(ip),
      button_event(button_event) {};
  std::string ip;
  ExternalButtonEvent button_event;
};

std::ostream& operator<<(std::ostream& os, const NewConnectionEvent& event);
std::ostream& operator<<(std::ostream& os, const LostNetworkEvent& event);
std::ostream& operator<<(std::ostream& os, const LostConnectionEvent&);
std::ostream& operator<<(std::ostream& os, const NetworkReceiveStateEvent& event);
