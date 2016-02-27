#pragma once

#include <vector>
#include <iostream>
#include <string>

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

