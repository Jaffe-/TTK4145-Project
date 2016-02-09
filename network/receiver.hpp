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
    Packet make_okay(Packet packet);
    int get_message_id(Packet packet);
    Socket& socket;
  };

}
