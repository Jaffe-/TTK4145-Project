#pragma once

#include "network_int.hpp"
#include <chrono>

class Network;

class Sender {
public:
  Sender(Network& network) : network(network) {};
  void send_message(const std::string& msg);
  void notify_okay(const std::string& ip, unsigned int id);
  void run();

private:
  Network& network;

  unsigned int current_id = 0;
  Packet make_packet(const std::string& msg, unsigned int id);
  const std::chrono::duration<double> send_timeout = std::chrono::seconds(1);
  friend std::ostream& operator<<(std::ostream& s, const MessageEntry& msg_entry);
};
