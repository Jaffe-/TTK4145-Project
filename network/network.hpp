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
    unsigned int id;
    std::vector<char> bytes;
    std::string ip;
  };

  
  void start(std::string port);
  void stop();
  std::string packet_type_name(Network::PacketType packet_type);
  double get_time();
  void run();
  
  extern Receiver* receiver;
  extern Sender* sender;
}


  
