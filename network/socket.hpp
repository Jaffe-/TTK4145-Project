#pragma once

//#include "network.hpp"
#include <iostream>
#include <vector>

struct Packet;

namespace Network {

  /* UDP socket abstraction */
  class Socket {
  public:
    Socket(std::string port);
    ~Socket();
    bool empty();
    Packet read();
    void write(Packet packet, std::string to_ip);

    std::string port;

  private:
    int sockfd;
    const int MAXBUF = 2048;
  };

}
