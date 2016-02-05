#pragma once

#include "network.hpp"
#include "socket.hpp"

#include <iostream>
#include <vector>

namespace Network {

  class Receiver {
  public:
    Receiver(Socket& socket) : socket(socket) {};
    void run();

  private:
    std::vector <std::string> buffer;
    Packet read();

    Socket& socket;
  };

}
