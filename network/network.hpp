#pragma once

#include <string>
#include "network_int.hpp"
#include "sender.hpp"
#include "receiver.hpp"
#include "connection_controller.hpp"
#include "socket.hpp"

class Network {
  friend class Sender;
  friend class Receiver;
  friend class ConnectionController;

public:
  Network(const std::string& port);
  void run();
  void send_message(const std::string& msg, unsigned int queue);
  unsigned int allocate_queue();

private:
  void send(const Packet& packet, const std::string& ip);
  void send_all(const Packet& packet);
  void broadcast(const Packet& packet);

  Socket socket;
  Sender sender;
  ConnectionController connection_controller;
  Receiver receiver;
};


  
