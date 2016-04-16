#include "network.hpp"
#include "sender.hpp"
#include "connection_manager.hpp"
#include "../util/logger.hpp"
#include <algorithm>

std::ostream& operator<<(std::ostream& s, const MessageEntry& msg_entry);

void Sender::send_message(const std::string& ip, const std::string& msg)
{
  if (network.connections.empty()) {
    return;
  }

  auto msg_entry = MessageEntry(current_id, msg);

  for (auto& connection : network.connections) {
    if (ip == "all" || ip == connection.first) {
      connection.second.message_queue.push_back(msg_entry);
      LOG(5, "Putting message in queue for " << ip);
    }
  }

  LOG(4, "New message " << msg_entry);
  current_id++;
}

void Sender::notify_okay(const std::string& ip, unsigned int id)
{
  std::vector<MessageEntry>& message_queue = network.connections[ip].message_queue;
  if (!message_queue.empty()) {
    MessageEntry& current = message_queue[0];
    if (current.id == id) {
      LOG(4, "OK received from " << ip
	  << " for message id " << current.id);

      message_queue.erase(message_queue.begin());
    }
  }
}

void Sender::check_timeout(const std::string& ip, MessageEntry& msg_entry, std::vector<std::string>& timed_out)
{
  if (std::chrono::system_clock::now() - msg_entry.sent_time > send_timeout) {
    LOG_WARNING("Client " << ip
		<< " did not respond with OK for message with id " << msg_entry.id);
    timed_out.push_back(ip);
  }
}

void Sender::send(const std::string& ip, MessageEntry& msg_entry)
{
  std::vector<char> bytes(msg_entry.msg.begin(), msg_entry.msg.end());

  auto packet = make_msg(msg_entry.id, bytes);
  network.send(packet, ip);
  msg_entry.sent = true;
  msg_entry.sent_time = std::chrono::system_clock::now();
}

void Sender::run()
{
  std::vector<std::string> timed_out;

  for (auto& conn : network.connections) {
    const std::string& ip = conn.first;
    std::vector<MessageEntry>& message_queue = conn.second.message_queue;
    if (!message_queue.empty()) {
      MessageEntry& current = message_queue[0];
      if (current.sent) {
	check_timeout(ip, current, timed_out);
      }
      else {
	send(ip, current);
      }
    }
  }

  network.connection_manager.remove_connections(timed_out);
}

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
    << "}";

  return s;
}
