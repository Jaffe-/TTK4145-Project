#pragma once

#include "network.hpp"

namespace Network {
  class Sender{
  public:
    void send(Packet packet);
    void notify_okay(int id);
    void send(Message msg);
  };
}
