#pragma once

#include "network.hpp"

#include <iostream>
#include <vector>

class Receiver {
public:
  Receiver(std::string port);
  ~Receiver();
  void run();

private:
  int sockfd;
  std::string port;
  std::vector <std::string> buffer;
  bool receive();
  Packet read();
};
