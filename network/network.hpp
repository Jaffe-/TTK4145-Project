#pragma once

#include <string>
#include "network_int.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "socket.hpp"
#include "../util/message_queue.hpp"

class Network {
  friend class Sender;
  friend class ConnectionController;

public:
  Network(const std::string& port);
  void run();
  MessageQueue message_queue;

private:
  void send(const Packet& packet, const std::string& ip);
  void send_all(const Packet& packet);
  void broadcast(const Packet& packet);
  void receive();
  
  Socket socket;
  Sender sender;
  ConnectionController connection_controller;

  struct connection {
    TimePoint last_ping;
    std::vector<MessageEntry> message_queue;
  };

  std::map<std::string, connection> connections;  
};


  
