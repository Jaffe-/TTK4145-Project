#include "network.hpp"
#include "sender.hpp"
#include "connection_controller.hpp"
#include "../util/logger.hpp"
#include <algorithm>

std::string truncate(const std::string& str, const std::string::size_type trunc_limit)
{
  if (str.size() <= trunc_limit)
    return str;
  else {
    std::string truncated = str.substr(0, std::min(str.length(), trunc_limit));
    std::replace(truncated.begin(), truncated.end(), '\n', ' ');
    return truncated + " (...)";
  }
}

std::ostream& operator<<(std::ostream& s, const MessageEntry& msg_entry)
{
  const std::string::size_type trunc_limit = 20;
  s << "{id=" << msg_entry.id
    << " sent=" << (msg_entry.sent ? "yes" : "no")
    << " msg=" << truncate(msg_entry.msg, trunc_limit)
    << " recipients=" << msg_entry.recipients
    << "}";

  return s;
}
  
void Sender::send_message(const std::string& msg)
{
  auto clients = network.connection_controller.get_clients();
  if (clients.empty()) {
    LOG_WARNING("Attempted to send message, but there are no clients.");
    return;
  }
  
  auto msg_entry = MessageEntry(current_id, msg, clients);

  for (auto& connection : network.connections) {
    connection.second.message_queue.push_back(msg_entry);
  }

  LOG_DEBUG("New message " << msg_entry);
  current_id++;
}

void Sender::notify_okay(const std::string& ip, unsigned int id)
{
  std::vector<MessageEntry>& message_queue = network.connections[ip].message_queue;
  if (!message_queue.empty()) {
    MessageEntry& current = message_queue[0];
    if (current.id == id) {
      // event
      LOG_DEBUG("OK received from " << ip
		<< " for message id " << current.id);

      message_queue.erase(message_queue.begin());
    }
  }
}

Packet Sender::make_packet(const std::string& msg, unsigned int id)
{
  std::vector<char> bytes(msg.begin(), msg.end());

  return make_msg(id, bytes);
}

void Sender::run()
{
  for (auto& kv : network.connections) {
    const std::string& ip = kv.first;
    std::vector<MessageEntry>& message_queue = kv.second.message_queue;
    if (!message_queue.empty()) {
      MessageEntry& current = message_queue[0];
      if (current.sent) {
	if (std::chrono::system_clock::now() - current.sent_time > send_timeout) {
	  // generate event that the message was not received by
	  // the IPs in current.recipients
	  LOG_WARNING("Clients " << current.recipients 
		      << " did not respond with OK for message with id " << current.id);
	  network.connection_controller.remove_clients(current.recipients);
	}
      }
      else {
	auto packet = make_packet(current.msg, current.id);
	network.send(packet, ip);
	current.sent = true;
	current.sent_time = std::chrono::system_clock::now();
      }
    }
  }
}



