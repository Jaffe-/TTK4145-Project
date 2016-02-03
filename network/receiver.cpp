#include "receiver.hpp"

#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 3
#define MAXBUF 2048

Receiver::Receiver(std::string port)
{
  struct addrinfo hints, *res;
  int rv;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
    //writeLogg()
    
  }

  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
    //fail!!
  }

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    close(sockfd);
    //fail
  }

  freeaddrinfo(res);

  this->port = port;
}

Receiver::Packet Receiver::receive()
{
  int numbytes;
  socklen_t addr_len;
  struct sockaddr_storage their_addr;
  char buf[MAXBUF];
  addr_len = sizeof(their_addr);

  if ((numbytes = recvfrom(sockfd, buf, MAXBUF-1, 0,
			   (struct sockaddr *)&their_addr, &addr_len)) == -1) {
    //fail
  }

  char s[INET_ADDRSTRLEN];
  inet_ntop(their_addr.ss_family, &(((struct sockaddr_in *)&their_addr)->sin_addr), s, sizeof(s));

  return {(PacketType)buf[0], std::string(buf+1), std::string(s)};
}

void Receiver::run()
{
  Packet packet = receive();
  switch (packet.type){
  case PacketType::PONG:
    break;
  case PacketType::MSG:
    buffer.push_back(packet.bytes);
    break;
  default:
    break;
  }
  std::cout << "Mottok " << p.bytes << " fra " << p.ip << std::endl;
}

Receiver::~Receiver()
{
  close(sockfd);
}
