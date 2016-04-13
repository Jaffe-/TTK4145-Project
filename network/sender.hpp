#pragma once

#include "network_int.hpp"
#include <chrono>

class Network;

class Sender {
public:
  Sender(Network& network) : network(network) {};
  void send_message(const std::string& ip, const std::string& msg);
  void notify_okay(const std::string& ip, unsigned int id);
  void run();

private:
  Network& network;

  unsigned int current_id = 0;
  const std::chrono::duration<double> send_timeout = std::chrono::seconds(1);

  void check_timeout(const std::string& ip, MessageEntry& msg, std::vector<std::string>& timed_out);
  void send(const std::string& ip, MessageEntry& msg_entry);

  friend std::ostream& operator<<(std::ostream& s, const MessageEntry& msg_entry);
};
