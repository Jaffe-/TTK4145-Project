#pragma once

#include <string>

namespace Network {
  enum class PacketType {
    PONG, MSG
  };

  struct Packet {
    PacketType type;
    std::string bytes;
    std::string ip;
  };

  struct Message {
    int id;
    std::string data;
  };

  int sockfd = 0;

  void start(std::string port);
  void stop();
  std::string packet_type_name(Network::PacketType packet_type);
}


  
