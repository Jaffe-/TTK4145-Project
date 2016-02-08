#include "network.hpp"
#include "receiver.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"

namespace Network {

  Receiver* receiver;
  Sender* sender;

  void start(std::string port)
  {
    static Socket socket(port);
    if (socket.operational) {
      receiver = new Receiver(socket);
      sender = new Sender(socket);
    }
    else {
      LOG_ERROR("Failed to create socket.");
      // ??
      return;
    }
  }

  void stop()
  {
    delete receiver;
    delete sender;
  }

  std::string packet_type_name(PacketType packet_type)
  {
    switch (packet_type) {
    case Network::PacketType::PING:
      return "PING";
    case Network::PacketType::PONG:
      return "PONG";
    case Network::PacketType::MSG:
      return "MSG";
    case Network::PacketType::OK:
      return "OK";
    default:
      return "Unused";
    }
  }
}
