#pragma once

#include <vector>
#include <string>

namespace Network {
  class Receiver;
  class Sender;

  enum class PacketType {
    PING, PONG, MSG, OK
  };

  struct Packet {
    PacketType type;
    std::vector<char> bytes;
    std::string ip;
  };

  struct Message {
    int id;
    std::string data;
  };
  
  void start(std::string port);
  void stop();
  std::string packet_type_name(Network::PacketType packet_type);
  double get_time();
  
  extern Receiver* receiver;
  extern Sender* sender;
}


  
