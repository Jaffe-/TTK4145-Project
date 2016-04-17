#pragma once

#include <string>
#include "network_int.hpp"
#include "sender.hpp"
#include "connection_manager.hpp"
#include "socket.hpp"
#include "../util/event_queue.hpp"
#include "events.hpp"
#include "../driver/events.hpp"
#include "../dispatch_logic/events.hpp"
#include "../util/logger.hpp"

class Network {
  friend class Sender;
  friend class ConnectionManager;

public:
  Network(EventQueue& logic_queue, const std::string& port);

  void run();
  std::string own_ip() {
    return socket.own_ips[0];
  }

  template <typename EventType>
  void notify(const NetworkMessageEvent<EventType>& event) {
    json_t json = {{"type", typeid(EventType).name()},
		   {"data", event.get_json()["data"]}};
    LOG(5, json.dump());
    sender.send_message(event.ip, json.dump());
  }

  EventQueue event_queue;

private:

  /* The events that are handled by this module */
  EventList<NetworkMessageEvent<StateUpdateEvent>,
	    NetworkMessageEvent<UpdateRequestEvent>,
	    NetworkMessageEvent<OrderCompleteEvent>,
	    NetworkMessageEvent<OrderTakenEvent>,
	    NetworkMessageEvent<OrderMapUpdateEvent>> events;

  void send(const Packet& packet, const std::string& ip);
  void send_all(const Packet& packet);
  void broadcast(const Packet& packet);
  void receive();
  void make_receive_event(const Packet& packet);

  EventQueue& logic_queue;
  Socket socket;
  Sender sender;
  ConnectionManager connection_manager;

  struct connection {
    TimePoint last_ping;
    std::vector<MessageEntry> message_queue;
  };

  std::map<std::string, connection> connections;

  /* Loop recursively through the event list and check whether the typeid of
     that event type matches the received event type, in which case we construct
     a new instance of that event and push it onto the event queue. */
  template <typename EventType, typename... Rest>
  void push_deserialized_event(const json_t& json, const std::string& ip,
			       EventList<NetworkMessageEvent<EventType>, Rest...>) {
    if (json["type"] == typeid(EventType).name())
      logic_queue.push(NetworkMessageEvent<EventType> {ip, EventType {json["data"]}});
    else
      push_deserialized_event(json, ip, EventList<Rest...>{});
  }

  /* The base case reached when the event list is empty, which means that none
     of the events were recognized. */
  void push_deserialized_event(const json_t&, const std::string&, EventList<>) {
    assert(false && "Unknown event received from network");
  }

};
