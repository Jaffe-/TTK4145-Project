#include "network.hpp"
#include "receiver.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"

namespace Network {

  Receiver* receiver;
  Sender* sender;

  void start(std::string port)
  {
    static Socket socket(port);
    receiver = new Receiver(socket);
    sender = new Sender(socket);
  }

  void stop()
  {
    delete receiver;
    delete sender;
  }

  std::string packet_type_name(PacketType packet_type)
  {
    switch (packet_type) {
    case Network::PacketType::PONG:
      return "PONG";
    case Network::PacketType::MSG:
      return "MSG";
    default:
      return "Unused";
    }
  }
}
