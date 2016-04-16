#include <string>
#include <iostream>
#include "network.hpp"
#include "sender.hpp"
#include "connection_manager.hpp"
#include "../util/logger.hpp"
#include <chrono>

Network::Network(EventQueue& logic_queue, const std::string& port)
  : logic_queue(logic_queue),
    socket(port),
    sender(*this),
    connection_manager(*this)
{
  LOG_INFO("Network started on port " << port);
}

void Network::run()
{
  while (true) {
    event_queue.handle_events(event_queue.acquire(), this, events);

    receive();
    sender.run();
    connection_manager.run();
  }
}

void Network::send(const Packet& packet, const std::string& ip)
{
  if (!socket.write(packet, ip)) {
    LOG(6, "Failed to write to socket");
  }
}

void Network::send_all(const Packet& packet)
{
  for (auto& ip: connection_manager.get_clients()) {
    send(packet, ip);
  }
}

void Network::broadcast(const Packet& packet)
{
  send(packet, "255.255.255.255");
}

void Network::make_receive_event(const Packet& packet)
{
  std::string serialized(packet.bytes.begin(), packet.bytes.end());

  push_deserialized_event(json_t::parse(serialized), packet.ip, events);
}

void Network::receive()
{
  if (socket.empty())
    return;

  Packet packet;
  if (!socket.read(packet)) {
    LOG(6, "Unable to read from socket");
    return;
  }

  if (socket.own_ip(packet.ip))
    return;

  LOG(6, "Received packet " << packet);

  connection_manager.notify_receive(packet);

  switch (packet.type){
  case PacketType::PING:
    send(make_pong(), packet.ip);
    break;
  case PacketType::MSG:
    send(make_okay(packet), packet.ip);
    make_receive_event(packet);
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

Packet make_okay(const Packet& packet)
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

std::ostream& operator<<(std::ostream& os, const NewConnectionEvent& event)
{
  return os << "{NewConnectionEvent " << event.ip << "}";
}

std::ostream& operator<<(std::ostream& os, const LostConnectionEvent& event)
{
  return os << "{LostConnectionEvent " << event.ip << "}";
}
