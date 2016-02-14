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
  std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& v);
  std::ostream& operator<<(std::ostream& s, const Packet& packet);

  void send(const Packet& packet, const std::string& ip);
  void send_all(const Packet& packet);
  void broadcast(const Packet& packet);

  Packet make_okay(Packet packet);
  Packet make_pong();
  Packet make_ping();
  Packet make_msg(unsigned int id, const std::vector<char>& bytes);

  extern Receiver* receiver;
  extern Sender* sender;

}
