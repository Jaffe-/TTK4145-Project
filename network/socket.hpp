#pragma once

#include "network_int.hpp"
#include <iostream>
#include <vector>

constexpr int MAXBUF = 20000;

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

  std::vector<std::string> own_ips;

private:
  int sockfd;
};
