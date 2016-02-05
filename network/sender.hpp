#pragma once

#include "network.hpp"
#include "socket.hpp"

namespace Network {

  class Sender{
  public:
    Sender(Socket& socket) : socket(socket) {};
    void send(Packet packet);
    void send_message(std::string msg);
    void notify_okay(int id);

  private:
    Socket& socket;
  };

}
