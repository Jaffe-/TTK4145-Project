#include "network_int.hpp"
#include "receiver.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"

namespace Network {

  Socket* socket;
  Receiver* receiver;
  Sender* sender;

  void start(std::string port)
  {
    socket = new Socket(port);
    if (socket->operational) {
      receiver = new Receiver(*socket);
      sender = new Sender(*socket);
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
    delete socket;
  }

  void run()
  {
    double t = get_time();
    int q = sender->allocate_queue();
    while (true) {
      receiver->run();
      sender->run();
      connection_controller.run();

      if (get_time() - t > 1) {
	sender->send_message("Test!\n", q);
	t = get_time();
      }
    }
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

  double get_time()
  {
    return static_cast<double>(std::clock())/CLOCKS_PER_SEC;
  }

  void send(const Packet& packet, const std::string& ip)
  {
    socket->write(packet, ip);
  }

  void send_all(const Packet& packet)
  {
    for (auto& ip: connection_controller.get_clients()) {
      send(packet, ip);
    }
  }

  void broadcast(const Packet& packet)
  {
    socket->write(packet, "255.255.255.255");
  }

}
