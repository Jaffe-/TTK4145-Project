#pragma once

#include <iostream>
#include <vector>

class Receiver {
public:
  Receiver(std::string port);
  void run();

private:
  int sockfd;
  std::string port;
  std::vector <std::string> buffer;
  std::string receive();
};
