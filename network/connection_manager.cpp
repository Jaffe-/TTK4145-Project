#include "network.hpp"
#include "connection_manager.hpp"
#include <vector>
#include "../util/logger.hpp"
#include <algorithm>
#include <chrono>
#include "events.hpp"

bool ConnectionManager::has_connection(const std::string& ip) const
{
  return network.connections.find(ip) != network.connections.end();
}

void ConnectionManager::notify_receive(const Packet& packet)
{
  // If we receive something from an ip that is not a registered connection,
  // we add it in the connections map and send an event to the dispatch logic
  // module.
  if (!has_connection(packet.ip)) {
    network.connections[packet.ip] = {std::chrono::system_clock::now(), {}};
    network.logic_queue.push(NewConnectionEvent(packet.ip));
    LOG_INFO("New connection " << packet.ip << " discovered");
  }

  if (packet.type == PacketType::PONG) {
    network.connections[packet.ip] = {std::chrono::system_clock::now(), {}};
  }
}

void ConnectionManager::remove_connections(const std::vector<std::string>& ips)
{
  for (auto& ip : ips) {
    if (has_connection(ip)) {
      network.logic_queue.push(LostConnectionEvent(ip));
      LOG_WARNING("Connection " << ip << " is removed");
      network.connections.erase(ip);
    }
  }
}

void ConnectionManager::check_timeouts()
{
  std::vector<std::string> timed_out;
  for (auto& kv : network.connections) {
    if (std::chrono::system_clock::now() - kv.second.last_ping > timeout_limit) {
      LOG_WARNING(kv.first << " stopped responding to PING");
      timed_out.push_back(kv.first);
    }
  }

  remove_connections(timed_out);
}

void ConnectionManager::send_ping()
{
  TimePoint now = std::chrono::system_clock::now();
  if (now - last_ping >= ping_period) {
    network.broadcast({PacketType::PING, 0, {}, ""});
    last_ping = now;
  }
}

void ConnectionManager::run()
{
  check_timeouts();
  send_ping();
}

std::vector<std::string> ConnectionManager::get_connections() const
{
  std::vector<std::string> results;

  for (auto& kv: network.connections) {
    results.push_back(kv.first);
  }

  return results;
}
