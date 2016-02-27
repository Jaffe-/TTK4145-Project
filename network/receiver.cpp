#include "network.hpp"
#include "receiver.hpp"
#include "../util/logger.hpp"
#include "connection_controller.hpp"
#include "sender.hpp"

void Receiver::run()
{
  if (network.socket.empty())
    return;

  Packet packet;
  if (!network.socket.read(packet)) {
    LOG_ERROR("Unable to read from socket");
    return;
  }

  if (network.socket.own_ip(packet.ip))
    return;

  LOG(4, "Received packet " << packet);

  switch (packet.type){
  case PacketType::PING:
    network.send(make_pong(), packet.ip);
    break;
  case PacketType::PONG:
    network.connection_controller.notify_pong(packet.ip);
    break;
  case PacketType::MSG:
    buffer.push_back(std::string(packet.bytes.begin(),
				 packet.bytes.end()));
    network.send(make_okay(packet), packet.ip);
    std::cout << buffer[buffer.size() - 1];
    break;
  case PacketType::OK:
    network.sender.notify_okay(packet.ip, packet.id);
    break;
  default:
    break;
  }
}
