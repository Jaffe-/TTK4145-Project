#pragma once

#include <string>
#include "network_int.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "socket.hpp"
#include "../util/event_queue.hpp"
#include "network_events.hpp"

class Network {
  friend class Sender;
  friend class ConnectionController;

public:
  Network(EventQueue& logic_queue, const std::string& port);
  void run();
  EventQueue event_queue;
  EventQueue& logic_queue;

private:
  void send(const Packet& packet, const std::string& ip);
  void send_all(const Packet& packet);
  void broadcast(const Packet& packet);
  void receive();
  void make_receive_event(const Packet& packet);

  Socket socket;
  Sender sender;
  ConnectionController connection_controller;

  struct connection {
    TimePoint last_ping;
    std::vector<MessageEntry> message_queue;
  };

  std::map<std::string, connection> connections;

  template <typename EventType>
  bool push_receive_event(const std::string& ip, const json_t& json) {
    if (json["type"] == typeid(EventType).name()) {
      logic_queue.push(NetworkReceiveEvent<EventType>{ip, EventType {json["data"]}});
      return true;
    }
    return false;
  }
};
