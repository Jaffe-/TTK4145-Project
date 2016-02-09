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

    if (socket.own_ip(packet.ip))
      return;
    
    switch (packet.type){
    case PacketType::PING:
      sender->send(make_pong(), packet.ip);
      break;
    case PacketType::PONG:
      connection_controller.notify_pong(packet.ip);
      break;
    case PacketType::MSG:
      buffer.push_back(std::string(packet.bytes.begin(),
				   packet.bytes.end()));
      sender->send(make_okay(packet), packet.ip);
      break;
    case PacketType::OK:
      sender->notify_okay(packet.ip, get_message_id(packet));
      break;
    default:
      break;
    }
    LOG_DEBUG("Received packet " << packet_type_name(packet.type)
	      << " (length " << packet.bytes.size()
	      << ") from " << packet.ip);
  }
    
  Packet Receiver::make_okay(Packet packet)
  {
    return { PacketType::OK,
	     std::vector<char>(packet.bytes.begin() + 1,
			       packet.bytes.begin() + 1 + sizeof(int)),
	""}; 
  }

  Packet Receiver::make_pong()
  {
    return { PacketType::PONG, {}, "" };
  }
  
  int Receiver::get_message_id(Packet packet)
  {
    char bytes[sizeof(int)];
    std::copy(packet.bytes.begin(), packet.bytes.begin() + sizeof(int), bytes);

    return *(int*)bytes;
  }
}
