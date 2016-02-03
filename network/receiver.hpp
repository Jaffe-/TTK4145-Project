#pragma once

#include <iostream>
#include <vector>

class Receiver {
public:
  Receiver(std::string port);
  ~Receiver();
  void run();

private:
  enum class PacketType {
    PONG, MSG
  };
  struct Packet {
    PacketType type;
    std::string bytes;
    std::string ip;
  };

  int sockfd;
  std::string port;
  std::vector <std::string> buffer;
  Packet receive();
};
