#include "receiver.hpp"
#include "../logger/logger.hpp"
#include "connection_controller.hpp"

namespace Network {

  void Receiver::run()
  {
    if (socket.empty())
      return;

    Packet packet = socket.read();
    switch (packet.type){
    case PacketType::PING:
      break;
    case PacketType::PONG:
      connection_controller.notify_pong(packet.ip);
      break;
    case PacketType::MSG:
      buffer.push_back(std::string(packet.bytes.begin(),
				   packet.bytes.end()));
      break;
    case PacketType::OK:
      break;
    default:
      break;
    }
    LOG_DEBUG("Received packet " << packet_type_name(packet.type)
	      << " (length " << packet.bytes.size()
	      << ") from " << packet.ip);
  }
}
