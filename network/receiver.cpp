#include "receiver.hpp"

#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BACKLOG 3
#define MAXBUF 2048

Receiver::Receiver(std::string port)
{
  struct addrinfo hints, *res;
  int rv;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
    //writeLogg()
    
  }

  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
    //fail!!
  }
  
  int yes = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    //fail
  } 

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    close(sockfd);
    //fail
  }

  freeaddrinfo(res);

  this->port = port;
}

std::string Receiver::receive()
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

  return std::string(buf);
}
