#pragma once

#include "network.hpp"
#include "socket.hpp"


namespace Network {

  class Sender{
  public:
    Sender(Socket& socket) : socket(socket), current_id(0) {};
    void send(Packet packet, std::string ip);
    void send_all(Packet packet);
    void broadcast(Packet packet);
    void send_message(std::string msg, int queue_id);
    void notify_okay(std::string ip, int id);
    int allocate_queue();
    void run();

  private:
    Socket& socket;
    struct MessageEntry{
      double sent_time;
      bool sent;
      Message msg;
      std::vector<std::string> recipients;
    };
    std::vector< std::vector<MessageEntry> > message_queues;
    int current_id;
    Packet make_packet(Message msg);
    const double send_timeout = 1;
  };

}
