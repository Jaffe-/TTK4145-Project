#include "network.hpp"
#include "../logger/logger.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Network {
  void start(std::string port)
  {
    struct addrinfo hints, *res;
    int rv;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
      LOG_ERROR("getaddrinfo() failed.");    
    }

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
      LOG_ERROR("socket() failed");
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
      close(sockfd);
      LOG_ERROR("bind() failed.");
    }

    freeaddrinfo(res);
}

  void stop()
  {
    close(sockfd);
  }

  std::string packet_type_name(PacketType packet_type)
  {
    switch (packet_type) {
    case Network::PacketType::PONG:
      return "PONG";
    case Network::PacketType::MSG:
      return "MSG";
    default:
      return "Unused";
    }
  }
}
