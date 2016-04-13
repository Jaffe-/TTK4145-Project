#pragma once

#include <string>
#include "network_int.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "socket.hpp"
#include "../util/event_queue.hpp"
#include "events.hpp"

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

  /* Loop recursively through the event list and check whether the typeid of
     that event type matches the received event type, in which case we construct
     a new instance of that event and push it onto the event queue. */
  template <typename EventType, typename... Rest>
  void push_receive_event(const json_t& json, const std::string& ip,
			  EventList<EventType, Rest...>) {
    if (json["type"] == typeid(NetworkMessageEvent<EventType>).name())
      logic_queue.push(NetworkMessageEvent<EventType>{ip, EventType {json["data"]}});
    else
      push_receive_event(json, ip, EventList<Rest...>{});
  }

  /* The base case reached when the event list is empty, which means that none
     of the events were recognized. */
  void push_receive_event(const json_t&, const std::string&, EventList<>) {
    assert(false && "Unknown event received from network");
  }

};
