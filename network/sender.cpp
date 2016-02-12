#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"
#include <algorithm>

namespace Network {
  
  std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& v)
  {
    stream << "[";
    for (auto& s : v) {
      stream << s << ", ";
    }
    stream << "\b\b]";
    return stream;
  }

  std::ostream& operator<<(std::ostream& s, const Sender::MessageEntry& msg_entry)
  {
    const std::string::size_type trunc_limit = 20;
    s << "{id=" << msg_entry.id
      << " sent=" << (msg_entry.sent ? "yes" : "no")
      << " msg=" << msg_entry.msg.substr(0, std::min(msg_entry.msg.length(), trunc_limit))
      << " recipients=" << msg_entry.recipients
      << "}";

    return s;
  }
  
  void Sender::send_message(const std::string& msg, int queue_id)
  {
    auto clients = connection_controller.get_clients();
    if (clients.empty()) {
      LOG_DEBUG("Attempted to send message, but there are no clients.");
      return;
    }

    auto msg_entry = MessageEntry(current_id, msg, clients);
    message_queues[queue_id].push_back(msg_entry);
    LOG_DEBUG("New message " << msg_entry 
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
    std::vector<char> bytes(msg.begin(), msg.end());

    return make_msg(id, bytes);
  }

  void Sender::run()
  {
    for (auto& message_queue : message_queues) {
      if (!message_queue.empty()) {
	MessageEntry& current = message_queue[0];
	if (current.sent) {
	  if (std::chrono::system_clock::now() - current.sent_time > send_timeout) {
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
	  current.sent_time = std::chrono::system_clock::now();
	}
      }
    }
  }
}


