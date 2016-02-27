#include <string>
#include <iostream>
#include "network.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "../util/logger.hpp"
#include <chrono>

Network::Network(const std::string& port) : socket(port), sender(*this), connection_controller(*this)
{
  LOG_INFO("Network started on port " << port);
}

void Network::run()
{
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

  TimePoint t = std::chrono::system_clock::now();
  int q = sender.allocate_queue();
  while (true) {
    receive();
    sender.run();
    connection_controller.run();

    if (std::chrono::system_clock::now() - t > std::chrono::seconds(2)) {
      sender.send_message("Test!\n", q);
      t = std::chrono::system_clock::now();
    }
  }
}

void Network::send_message(const std::string& msg, unsigned int queue)
{
  sender.send_message(msg, queue);
}

unsigned int Network::allocate_queue()
{
  return sender.allocate_queue();
}

void Network::send(const Packet& packet, const std::string& ip)
{
  socket.write(packet, ip);
}

void Network::send_all(const Packet& packet)
{
  for (auto& ip: connection_controller.get_clients()) {
    send(packet, ip);
  }
}

void Network::broadcast(const Packet& packet)
{
  socket.write(packet, "255.255.255.255");
}

void Network::receive()
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

  LOG(4, "Received packet " << packet);

  switch (packet.type){
  case PacketType::PING:
    send(make_pong(), packet.ip);
    break;
  case PacketType::PONG:
    connection_controller.notify_pong(packet.ip);
    break;
  case PacketType::MSG:
    //    buffer.push_back(std::string(packet.bytes.begin(),
    //				 packet.bytes.end()));
    //network.send(make_okay(packet), packet.ip);
    //std::cout << buffer[buffer.size() - 1];
    break;
  case PacketType::OK:
    sender.notify_okay(packet.ip, packet.id);
    break;
  default:
    break;
  }
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
  case PacketType::PING:
    return "PING";
  case PacketType::PONG:
    return "PONG";
  case PacketType::MSG:
    return "MSG";
  case PacketType::OK:
    return "OK";
  default:
    return "Unused";
  }
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
