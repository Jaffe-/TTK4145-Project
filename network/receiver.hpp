#pragma once

#include "network.hpp"

#include <iostream>
#include <vector>

namespace Network {
  class Receiver {
  public:
    void run();

  private:
    std::vector <std::string> buffer;
    Packet read();
    bool receive();
  };

  extern Receiver receiver;
}
