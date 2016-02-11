#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"
#include <cstdlib>
#include <algorithm>

namespace Network {

  void Sender::send(const Packet& packet, const std::string& ip) const
  {
    socket.write(packet, ip, 0);
  }

  void Sender::send_all(const Packet& packet) const
  {
    for (auto& ip: connection_controller.get_clients()) {
      send(packet, ip);
    }
  }

  void Sender::broadcast(const Packet& packet) const
  {
    socket.write(packet, "255.255.255.255", 1);
  }
  
  std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& v)
  {
    stream << "[";
    for (auto& s : v) {
      stream << s << ", ";
    }
    stream << "\b]";
    return stream;
  }

  void Sender::send_message(const std::string& msg, int queue_id)
  {
    auto clients = connection_controller.get_clients();
    if (clients.empty())
      return;

    message_queues[queue_id].push_back({current_id, 0, false, msg, clients});
    LOG_DEBUG("New message with id " << current_id 
	      << " to " << clients
	      << " put in queue " << queue_id);
    current_id++;
  }

  void Sender::notify_okay(const std::string& ip, unsigned int id)
  {
    for (auto &message_queue : message_queues) {
      if (!message_queue.empty()) {
	MessageEntry& current = message_queue[0];
	if (current.id == id) {
	  // event
	  LOG_DEBUG("OK received from " << ip
		    << " for message id " << current.id);

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

  Packet Sender::make_packet(const std::string& msg, unsigned int id)
  {
    std::vector<char> bytes;

    std::copy(msg.begin(), msg.end(), std::back_inserter(bytes));

    return { PacketType::MSG,
	     id,
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
	    // generate event that the message was not received by
	    // the IPs in current.recipients
	    LOG_WARNING("Clients " << current.recipients 
			<< " did not respond with OK for message with id " << current.id);
	    connection_controller.remove_clients(current.recipients);
	    message_queue.erase(message_queue.begin());
	  }
	}
	else {
	  Packet packet = make_packet(current.msg, current.id);
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


