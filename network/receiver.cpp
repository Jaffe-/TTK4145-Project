#include "receiver.hpp"

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

  if (listen(sockfd, BACKLOG) == -1) {
    //fail
  }
}

