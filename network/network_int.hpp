#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

struct MessageEntry {
  MessageEntry(unsigned int id, const std::string& msg,
	       const std::vector<std::string>& recipients) :
    id(id), sent(false), msg(msg), recipients(recipients) {};
  
  unsigned int id;
  TimePoint sent_time;
  bool sent;
  std::string msg;
  std::vector<std::string> recipients;
};

enum class PacketType {
  PING, PONG, MSG, OK
};

struct Packet {
  PacketType type;
  unsigned int id;
  std::vector<char> bytes;
  std::string ip;
};

std::string packet_type_name(PacketType packet_type);
std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& v);
std::ostream& operator<<(std::ostream& s, const Packet& packet);

Packet make_okay(Packet packet);
Packet make_pong();
Packet make_ping();
Packet make_msg(unsigned int id, const std::vector<char>& bytes);

