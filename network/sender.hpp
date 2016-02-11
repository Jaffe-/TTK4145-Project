#pragma once

#include "network_int.hpp"
#include "socket.hpp"

namespace Network {

  class Sender {
  public:
    Sender(Socket& socket) : socket(socket), current_id(0) {};
    void send(const Packet& packet, const std::string& ip) const;
    void send_all(const Packet& packet) const;
    void broadcast(const Packet& packet) const;
    void send_message(const std::string& msg, int queue_id);
    void notify_okay(const std::string& ip, unsigned int id);
    int allocate_queue();
    void run();

  private:
    Socket& socket;
    struct MessageEntry{
      unsigned int id;
      double sent_time;
      bool sent;
      std::string msg;
      std::vector<std::string> recipients;
    };
    std::vector< std::vector<MessageEntry> > message_queues;
    unsigned int current_id;
    Packet make_packet(const std::string& msg, unsigned int id);
    const double send_timeout = 1;
  };

}
