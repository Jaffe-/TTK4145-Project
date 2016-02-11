#pragma once

#include "network.hpp"
#include <vector>

namespace Network {

  class Receiver;
  class Sender;

  enum class PacketType {
    PING, PONG, MSG, OK
  };

  struct Packet {
    PacketType type;
    unsigned int id;
    std::vector<char> bytes;
    std::string ip;
  };

  std::string packet_type_name(Network::PacketType packet_type);
  void send(const Packet& packet, const std::string& ip);
  void send_all(const Packet& packet);
  void broadcast(const Packet& packet);
  double get_time();

  extern Receiver* receiver;
  extern Sender* sender;

}