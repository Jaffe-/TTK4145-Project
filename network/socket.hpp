#pragma once

#include "network_int.hpp"
#include <iostream>
#include <vector>

const int MAXBUF = 2048;

/* UDP socket abstraction */
class Socket {
public:
  Socket(const std::string& port);
  ~Socket();
  bool empty();
  bool read(Packet& packet);
  bool write(const Packet& packet, const std::string& to_ip);
  bool own_ip(const std::string& ip);

  std::string port;
  bool operational;

private:
  int sockfd;
  std::vector<std::string> own_ips;
};
