#pragma once

#include "network_int.hpp"

#include <iostream>
#include <vector>

class Network;

class Receiver {
public:
  Receiver(Network& network) : network(network) {};
  void run();

private:
  std::vector <std::string> buffer;
  Packet read();
  int get_message_id(Packet packet);
  Network& network;
};
