#pragma once

#include <vector>
#include <string>

namespace Network {
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

  extern int sockfd;
  extern std::string port;
  
  void start(std::string port);
  void stop();
  std::string packet_type_name(Network::PacketType packet_type);
}


  
