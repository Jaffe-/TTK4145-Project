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

  void Sender::send_message(std::string msg) {
    union id_t {
      int val;
      char bytes[sizeof(int)];
    };
    static id_t id;

    std::vector<char> bytes;
    for (int i = 0; i < (int)sizeof(int); i++)
      bytes.push_back(id.bytes[i]);

    std::copy(msg.begin(), msg.end(), std::back_inserter(bytes));
    Packet packet = { PacketType::MSG,
		      bytes,
		      std::string() };

    id.val++;
  }

}
