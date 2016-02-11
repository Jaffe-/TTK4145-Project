#pragma once

#include "network_int.hpp"
#include "socket.hpp"
#include <chrono>

namespace Network {

  class Sender {
  public:
    Sender(Socket& socket) : socket(socket), current_id(0) {};
    void send_message(const std::string& msg, int queue_id);
    void notify_okay(const std::string& ip, unsigned int id);
    int allocate_queue();
    void run();

  private:
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    Socket& socket;
    struct MessageEntry{
      MessageEntry(unsigned int id, const std::string& msg,
		   const std::vector<std::string>& recipients) :
	id(id), sent(false), msg(msg), recipients(recipients) {};

      unsigned int id;
      TimePoint sent_time;
      bool sent;
      std::string msg;
      std::vector<std::string> recipients;
    };
    std::vector< std::vector<MessageEntry> > message_queues;
    unsigned int current_id;
    Packet make_packet(const std::string& msg, unsigned int id);
    const std::chrono::duration<double> send_timeout = std::chrono::seconds(1);
  };

}
