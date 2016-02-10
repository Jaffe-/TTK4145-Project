#pragma once

#include "network_int.hpp"
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
    bool write(Packet packet, std::string to_ip, bool broadcast);
    bool own_ip(std::string ip);

    std::string port;
    bool operational;

  private:
    int sockfd;
    std::vector<std::string> own_ips;
  };

}
