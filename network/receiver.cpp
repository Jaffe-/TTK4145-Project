#include "receiver.hpp"
#include "../logger/logger.hpp"
#include "connection_controller.hpp"
#include "sender.hpp"

namespace Network {

  void Receiver::run()
  {
    if (socket.empty())
      return;

    Packet packet;
    if (!socket.read(packet)) {
      LOG_ERROR("Unable to read from socket");
      return;
    }

    LOG(Logger::LogLevel::DEBUG_EXTRA, "Received packet " << packet);

    if (socket.own_ip(packet.ip))
      return;

    switch (packet.type){
    case PacketType::PING:
      send(make_pong(), packet.ip);
      break;
    case PacketType::PONG:
      connection_controller.notify_pong(packet.ip);
      break;
    case PacketType::MSG:
      buffer.push_back(std::string(packet.bytes.begin(),
				   packet.bytes.end()));
      send(make_okay(packet), packet.ip);
      std::cout << buffer[buffer.size() - 1];
      break;
    case PacketType::OK:
      sender->notify_okay(packet.ip, packet.id);
      break;
    default:
      break;
    }
  }
      
}
