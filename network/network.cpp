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
  while (true) {
    for (auto& msg : message_queue.take_messages(message_queue.acquire())) {
      if (msg->serializable()) {
	const Serializable& serializable_msg = *msg;
	std::string serialized = serializable_msg.serialize();
	LOG_DEBUG("Sending serializable message: " << serialized);
	send_message(serialized);
      }
      else
	LOG_DEBUG("Non serializble");
    }
    
    receive();
    sender.run();
    connection_controller.run();
    
  }
}

void Network::send_message(const std::string& msg)
{
  sender.send_message(msg);
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
    send(make_okay(packet), packet.ip);
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
