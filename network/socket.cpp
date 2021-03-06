#include "network_int.hpp"
#include "../util/logger.hpp"
#include "socket.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <algorithm>
#include "../util/init_exception.hpp"

Socket::Socket(const std::string& port) : port(port)
{
  addrinfo hints, *res;
  int rv;

  operational = false;

  /* Find list of own IPv4 addresses */
  ifaddrs * if_addrs=NULL;
  getifaddrs(&if_addrs);

  for (ifaddrs* ifa = if_addrs; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) {
      char address[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((sockaddr_in*)ifa->ifa_addr)->sin_addr, address, INET_ADDRSTRLEN);
      if (std::string(ifa->ifa_name) != "lo")
	own_ips.push_back(address);
    }
  }
  if (if_addrs != NULL)
    freeifaddrs(if_addrs);
  else {
    throw InitException("getifaddrs() failed");
  };

  if (own_ips.empty()) {
    throw InitException("No network interfaces found");
  }

  LOG_DEBUG("IPs of own interfaces: " << own_ips);

  /* Set up UDP socket with broadcasting enabled */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0) {
    throw InitException("getaddrinfo() failed");
  }

  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
    throw InitException("socket() failed");
  }

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    close(sockfd);
    throw InitException("bind() failed");
  }

  int b = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &b, sizeof(b)) == -1) {
    throw InitException("setsockopt() failed");
  }

  freeaddrinfo(res);
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
  timeval tv;
  const int select_timeout = 10;

  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = select_timeout;
  int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

  if (rv == -1) {
    LOG(6, "select() failed.");
    return true;
  }
  else if (rv == 0)
    return true;
  else
    return false;
}

bool Socket::read(Packet& packet)
{
  int numbytes;
  socklen_t addr_len;
  sockaddr_storage their_addr;
  char buf[MAXBUF] = {0};
  addr_len = sizeof(their_addr);

  if ((numbytes = recvfrom(sockfd, buf, MAXBUF-1, 0,
			   (sockaddr*)&their_addr, &addr_len)) == -1) {
    LOG(6, "recvfrom() failed.");
    return false;
  }

  char s[INET_ADDRSTRLEN];
  inet_ntop(their_addr.ss_family, &(((sockaddr_in *)&their_addr)->sin_addr), s, sizeof(s));
  packet = {(PacketType)buf[0],
	    *(unsigned int*) (buf + 1),
	    std::vector<char>(buf + 1 + sizeof(int), buf + numbytes),
	    std::string(s)};
  return true;
}

bool Socket::write(const Packet& packet, const std::string& to_ip)
{
  sockaddr_in si;
  char buf[MAXBUF];

  memset((char *) &si, 0, sizeof(si));
  si.sin_family = AF_INET;
  si.sin_port = htons(atoi(port.c_str()));
  inet_pton(AF_INET, to_ip.c_str(), &(si.sin_addr));

  buf[0] = static_cast<char>(packet.type);
  *(unsigned int*) (buf + 1) = packet.id;
  std::copy(packet.bytes.begin(), packet.bytes.end(), buf + 1 + sizeof(int));
  if (sendto(sockfd, buf, packet.bytes.size() + 1 + sizeof(int),
	     0, (sockaddr*) &si, sizeof(si)) == -1) {
    LOG(6, "sendto() failed");
    return false;
  }

  return true;
}

bool Socket::own_ip(const std::string& ip)
{
  return std::find(own_ips.begin(), own_ips.end(), ip) != own_ips.end();
}
