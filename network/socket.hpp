#pragma once

//#include "network.hpp"
#include <iostream>
#include <vector>

struct Packet;

namespace Network {

  const int MAXBUF = 2048;

  /* UDP socket abstraction */
  class Socket {
  public:
    Socket(std::string port);
    ~Socket();
    bool empty();
    bool read(Packet& packet);
    bool write(Packet& packet, std::string to_ip);

    std::string port;
    bool operational;

  private:
    int sockfd;
  };

}
