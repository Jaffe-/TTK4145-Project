#include "network.hpp"
#include "connection_controller.hpp"
#include <vector>
#include "../util/logger.hpp"
#include <algorithm>
#include <chrono>

bool ConnectionController::has_client(const std::string& ip) const
{
  return std::find_if(connections.begin(), connections.end(),
		      [&] (std::pair<std::string, TimePoint> p) 
		      { return p.first == ip; }) != connections.end();
}

void ConnectionController::notify_pong(const std::string& ip)
{
  if (!has_client(ip)) {
    // new client registered, send event
    LOG_INFO("New client " << ip << " discovered");
  }
  connections[ip] = std::chrono::system_clock::now();
}

void ConnectionController::remove_clients(const std::vector<std::string>& ips)
{
  for (auto& ip : ips) {
    if (has_client(ip)) {
      // send event
      LOG_WARNING("Client " << ip << " is removed");
      connections.erase(ip);
    }
  }
}

void ConnectionController::check_timeouts()
{
  std::vector<std::string> timed_out;
  for (auto& kv : connections) {
    if (std::chrono::system_clock::now() - kv.second > timeout_limit) {
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

  for (auto& kv: connections) {
    results.push_back(kv.first);
  }

  return results;
}
