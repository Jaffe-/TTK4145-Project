#pragma once

#include <string>
#include "../util/message_queue.hpp"

struct NewConnectionEvent : public Message {
  NewConnectionEvent(const std::string& ip) : ip(ip) {};
  std::string ip;
};

struct LostConnectionEvent : public Message {
  LostConnectionEvent(const std::string& ip) : ip(ip) {};
  std::string ip;
};

struct LostNetworkEvent : public Message {
  LostNetworkEvent() {};
};

std::ostream& operator<<(std::ostream& os, const NewConnectionEvent& event);
std::ostream& operator<<(std::ostream& os, const LostNetworkEvent& event);
std::ostream& operator<<(std::ostream& os, const LostConnectionEvent& event);
