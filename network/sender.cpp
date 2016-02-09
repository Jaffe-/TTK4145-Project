#include "sender.hpp"
#include "connection_controller.hpp"
#include "../logger/logger.hpp"
#include <cstdlib>

namespace Network {

  void Sender::send(Packet packet)
  {
    for (auto& ip: connection_controller.get_clients()) {
      socket.write(packet, ip);
    }
  }

  void Sender::send_message(std::string msg, int queue_id) {
    message_queues[queue_id].push_back({0, {current_id, msg}});
    current_id++;
  }

  void Sender::notify_okay(int id)
  {
    for (auto &message_queue : message_queues) {
      if (!message_queue.empty()
	  && message_queue[0].msg.id == id) {
	// event
	message_queue.erase(message_queue.begin());
      }
    }
  }

  int Sender::allocate_queue()
  {
    message_queues.push_back({});
    return message_queues.size() - 1;
  }

  Packet Sender::make_packet(Message msg)
  {
    char id_bytes[sizeof(int)];
    *(int*)id_bytes = msg.id;

    std::vector<char> bytes;

    std::copy(id_bytes, id_bytes + sizeof(id_bytes),
	      std::back_inserter(bytes));
    std::copy(msg.data.begin(), msg.data.end(),
	      std::back_inserter(bytes));
    return { PacketType::MSG,
	     bytes,
	     std::string() };
  }

  void Sender::run()
  {

  }
}


