#include "network.hpp"
#include "../logger/logger.hpp"
#include "socket.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Network {

  Socket::Socket(std::string port) : port(port)
  {
    struct addrinfo hints, *res;
    int rv;

    operational = false;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
      LOG_ERROR("getaddrinfo() failed.");
      return;
    }

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
      LOG_ERROR("socket() failed");
      return;
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
      close(sockfd);
      LOG_ERROR("bind() failed.");
      return;
    }

    freeaddrinfo(res);

    LOG_DEBUG("Created new socket on port " << port);

    operational = true;
  }

  Socket::~Socket()
  {
    close(sockfd);
    LOG_DEBUG("Closed socket on port " << port);
  }

  bool Socket::empty()
  {
    fd_set readfds;
    struct timeval tv;
    const int select_timeout = 10;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = select_timeout;
    int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

    if (rv == -1) {
      LOG_ERROR("select() failed.");
      // Todo: find out what to do in this case
      return true;
    }
    else if (rv == 0)
      return false;
    else
      return true;
  }

  bool Socket::read(Packet& packet)
  {
    int numbytes;
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    char buf[MAXBUF] = {0};
    addr_len = sizeof(their_addr);

    if ((numbytes = recvfrom(sockfd, buf, MAXBUF-1, 0,
			     (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      LOG_ERROR("recvfrom() failed.");
      return false;
    }

    char s[INET_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, &(((struct sockaddr_in *)&their_addr)->sin_addr), s, sizeof(s));

    packet = {(PacketType)buf[0],
	      std::vector<char>(buf + 1, buf + MAXBUF),
	      std::string(s)};
    return true;
  }

  bool Socket::write(Packet& packet, std::string to_ip)
  {
    struct sockaddr_in si;
    char buf[MAXBUF];

    memset((char *) &si, 0, sizeof(si));
    si.sin_family = AF_INET;
    si.sin_port = htons(atoi(port.c_str()));
    inet_pton(AF_INET, to_ip.c_str(), &(si.sin_addr));

    std::copy(packet.bytes.begin(), packet.bytes.end(), buf);
    if (sendto(sockfd, buf, packet.bytes.size(),
	       0, (struct sockaddr*) &si, sizeof(si)) == -1) {
      LOG_ERROR("sendto() failed");
      return false;
    }
    return true;
  }
}
