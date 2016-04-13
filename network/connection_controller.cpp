#include "network.hpp"
#include "connection_controller.hpp"
#include <vector>
#include "../util/logger.hpp"
#include <algorithm>
#include <chrono>
#include "events.hpp"

bool ConnectionController::has_client(const std::string& ip) const
{
  return network.connections.find(ip) != network.connections.end();
}

void ConnectionController::notify_receive(const Packet& packet)
{
  if (!has_client(packet.ip)) {
    network.connections[packet.ip] = {std::chrono::system_clock::now(), {}};
    network.logic_queue.push(NewConnectionEvent(packet.ip));
    LOG_INFO("New client " << packet.ip << " discovered");
  }
  if (packet.type == PacketType::PONG) {
    network.connections[packet.ip] = {std::chrono::system_clock::now(), {}};
  }
}

void ConnectionController::remove_clients(const std::vector<std::string>& ips)
{
  for (auto& ip : ips) {
    if (has_client(ip)) {
      network.logic_queue.push(LostConnectionEvent(ip));
      LOG_WARNING("Client " << ip << " is removed");
      network.connections.erase(ip);
    }
  }
}

void ConnectionController::check_timeouts()
{
  std::vector<std::string> timed_out;
  for (auto& kv : network.connections) {
    if (std::chrono::system_clock::now() - kv.second.last_ping > timeout_limit) {
      LOG_WARNING(kv.first << " stopped responding to PING");
      timed_out.push_back(kv.first);
    }
  }

  remove_clients(timed_out);
}

void ConnectionController::send_ping()
{
  TimePoint now = std::chrono::system_clock::now();
  if (now - last_ping >= ping_period) {
    network.broadcast({PacketType::PING, 0, {}, ""});
    last_ping = now;
  }
}

void ConnectionController::run()
{
  check_timeouts();
  send_ping();
}

std::vector<std::string> ConnectionController::get_clients() const
{
  std::vector<std::string> results;

  for (auto& kv: network.connections) {
    results.push_back(kv.first);
  }

  return results;
}
