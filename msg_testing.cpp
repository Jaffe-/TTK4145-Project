#include <iostream>
#include <map>
#include <functional>
#include <string>
#include <memory>
#include "util/message_queue.hpp"
#include <thread>
#include <chrono>

void sender1(MessageQueue& queue)
{
  unsigned int id = 0;
  while (true) {
    for (int i = 0; i < 5; i++) {
      std::string s = "1::Message number " + std::to_string(id++);
      auto msg = std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, s);
      queue.push(std::move(msg));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

void sender2(MessageQueue& queue)
{
  unsigned int id = 0;
  while(true) {
    queue.push(std::make_shared<Message<int>>(TMessage::WHATEVER, id++));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
}

void handler1(const Message<std::string>& s)
{
  std::cout << "STRING: " << s.data << std::endl;
}

void handler2(const Message<int>& s)
{
  std::cout << "INT: " << s.data << std::endl;
}

int main()
{
  MessageQueue Q;

  std::map<TMessage, std::function<void(const BaseMessage&)>> handlers = {
    {TMessage::NETWORK_SEND, handler1},
    {TMessage::WHATEVER, handler2}
  };

  std::thread t1(sender1, std::ref(Q));
  std::thread t2(sender2, std::ref(Q));

  /* Sleeping version */
  while (true) {
    for (const auto& msg : Q.take_messages(Q.wait())) {
      handlers[msg->type](*msg);
    }
  }

  /* Polling version */
  while (true) {
    for (const auto& msg : Q.take_messages(Q.acquire())) {
      handlers[msg->type](*msg);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}
