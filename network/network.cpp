#include "network_int.hpp"
#include "receiver.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "../util/logger.hpp"
#include <chrono>

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
      LOG_INFO("Network started on port " << port);
    }
    else {
      LOG_ERROR("Failed to start network");
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
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    TimePoint t = std::chrono::system_clock::now();
    int q = sender->allocate_queue();
    while (true) {
      receiver->run();
      sender->run();
      connection_controller.run();

      if (std::chrono::system_clock::now() - t > std::chrono::seconds(2)) {
	sender->send_message("Test!\n", q);
	t = std::chrono::system_clock::now();
      }
    }
  }

  void send_message(const std::string& msg, unsigned int queue)
  {
    sender->send_message(msg, queue);
  }

  unsigned int allocate_queue()
  {
    return sender->allocate_queue();
  }

  std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& v)
  {
    stream << "[";
    for (auto& s : v) {
      stream << s << ", ";
    }
    stream << "\b\b]";
    return stream;
  }

  std::ostream& operator<<(std::ostream& s, const Packet& packet)
  {
    s << "{" << packet_type_name(packet.type)
      << " id=" << packet.id
      << " length=" << packet.bytes.size()
      << " from=" << packet.ip
      << "}";
    return s;
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

  Packet make_okay(Packet packet)
  {
    return { PacketType::OK, packet.id,{}, ""}; 
  }

  Packet make_pong()
  {
    return { PacketType::PONG, 0, {}, "" };
  }

  Packet make_ping()
  {
    return { PacketType::PING, 0, {}, ""};
  }

  Packet make_msg(unsigned int id, const std::vector<char>& bytes)
  {
    return { PacketType::MSG, id, bytes, ""};
  }

}
