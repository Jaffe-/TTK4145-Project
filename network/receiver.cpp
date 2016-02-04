#include "receiver.hpp"
#include "../logger/logger.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Receiver::Receiver(std::string port)
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

  this->port = port;
}

bool Receiver::receive()
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
  }
  else if (rv == 0)
    return true;
  else
    return false;
}


#define MAXBUF 2048

Receiver::Packet Receiver::read()
{
  int numbytes;
  socklen_t addr_len;
  struct sockaddr_storage their_addr;
  char buf[MAXBUF];
  addr_len = sizeof(their_addr);

  if ((numbytes = recvfrom(sockfd, buf, MAXBUF-1, 0,
			   (struct sockaddr *)&their_addr, &addr_len)) == -1) {
    LOG_ERROR("recvfrom() failed.");
  }

  char s[INET_ADDRSTRLEN];
  inet_ntop(their_addr.ss_family, &(((struct sockaddr_in *)&their_addr)->sin_addr), s, sizeof(s));

  return {(PacketType)buf[0], std::string(buf+1), std::string(s)};
}

void Receiver::run()
{
  if (!receive())
    return;

  Packet packet = read();
  switch (packet.type){
  case PacketType::PONG:
    break;
  case PacketType::MSG:
    buffer.push_back(packet.bytes);
    break;
  default:
    break;
  }
  LOG_DEBUG("Received packet " << get_type_name(packet.type)
	    << " (length " << packet.bytes.size()
	    << ") from " << packet.ip);
}

Receiver::~Receiver()
{
  close(sockfd);
}
