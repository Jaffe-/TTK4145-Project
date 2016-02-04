#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"
#include <cstdlib>

#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "connection_controller.hpp"

namespace Network {

  void send(Packet packet, std::string to_ip) {
    struct sockaddr_in si;
    const int MAXBUF = 2048;
    char buf[MAXBUF];

    memset((char *) &si, 0, sizeof(si));
    si.sin_family = AF_INET;
    si.sin_port = htons(atoi(port.c_str()));
    inet_pton(AF_INET, to_ip.c_str(), &(si.sin_addr));

    std::copy(packet.bytes.begin(), packet.bytes.end(), buf);
    if (sendto(sockfd, buf, packet.bytes.size(),
	       0, (struct sockaddr*) &si, sizeof(si)) == -1)
      LOG_ERROR("sendto() failed");

  }

  void send(Packet packet)
  {
    for (auto& ip: connection_controller.get_clients()) {
      send(packet, ip);
    }
  }

  void send_message(std::string msg) {
    union id_t {
      int val;
      char bytes[sizeof(int)];
    };
    static id_t id;

    std::vector<char> bytes;
    for (int i = 0; i < sizeof(int); i++)
      bytes.push_back(id.bytes[i]);
    
    std::copy(msg.begin(), msg.end(), std::back_inserter(bytes));
    Packet packet = { PacketType::MSG,
		      bytes,
		      std::string() };
    
  }
  
}
