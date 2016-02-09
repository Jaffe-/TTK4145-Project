#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"
#include <cstdlib>
#include <algorithm>

namespace Network {

  void Sender::send(Packet packet, std::string ip)
  {
    socket.write(packet, ip);
  }

  void Sender::send_all(Packet packet)
  {
    for (auto& ip: connection_controller.get_clients()) {
      send(packet, ip);
    }
  }

  void Sender::broadcast(Packet packet)
  {
    send(packet, "255.255.255.255");
  }
  
  void Sender::send_message(std::string msg, int queue_id)
  {
    message_queues[queue_id].push_back({0, false, {current_id, msg},
	  connection_controller.get_clients()});
    LOG_DEBUG("New message with id " << current_id << " put in queue " << queue_id);
    current_id++;
  }

  void Sender::notify_okay(std::string ip, int id)
  {
    for (auto &message_queue : message_queues) {
      if (!message_queue.empty()) {
	MessageEntry current = message_queue[0];
	if (current.msg.id == id) {
	  // event
	  LOG_DEBUG("OK received for message id " << message_queue[0].msg.id);

	  auto it = std::find(current.recipients.begin(), current.recipients.end(), ip);
	  if (it != current.recipients.end())
	    current.recipients.erase(it);

	  if (current.recipients.empty())
	    message_queue.erase(message_queue.begin());
	}
      }
    }
  }

  int Sender::allocate_queue()
  {
    message_queues.push_back({});
    int queue_id = message_queues.size() - 1;
    LOG_DEBUG("New queue id " << queue_id << " allocated");
    return queue_id;
  }

  Packet Sender::make_packet(Message msg)
  {
    char id_bytes[sizeof(int)];
    *(int*)id_bytes = msg.id;

    std::vector<char> bytes;

    std::copy(id_bytes, id_bytes + sizeof(id_bytes), std::back_inserter(bytes));
    std::copy(msg.data.begin(), msg.data.end(), std::back_inserter(bytes));

    return { PacketType::MSG,
	     bytes,
	     std::string() };
  }

  void Sender::run()
  {
    for (auto& message_queue : message_queues) {
      if (!message_queue.empty()) {
	MessageEntry& current = message_queue[0];
	if (current.sent) {
	  if (get_time() - current.sent_time > send_timeout) {
	    // something failed, send event
	    connection_controller.remove_clients(current.recipients);
	  }
	}
	else {
	  Packet packet = make_packet(current.msg);
	  for (auto& ip : current.recipients) {
	    send(packet, ip);
	  }
	  current.sent = true;
	  current.sent_time = get_time();
	}
      }
    }
  }
}


