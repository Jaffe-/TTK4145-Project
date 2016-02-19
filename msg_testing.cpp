#include <iostream>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <memory>
#include "util/message_queue.hpp"
#include <thread>
#include <chrono>

void sender1(MessageQueue& queue)
{
  using namespace std::literals;
  for (int i = 0; i < 100000; i++) {
    std::string s = "1::Message number " + std::to_string(i);
    queue.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, s));
    std::this_thread::sleep_for(10ms);
  }
}

void sender2(MessageQueue& queue)
{
  using namespace std::literals;
  for (int i = 0; i < 100000; i++) {
    std::string s = "2::Message number " + std::to_string(i);
    queue.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, s));
    std::this_thread::sleep_for(10ms);
  }
}


/* Wraps a handler function taking Message<T> in a lambda which takes care of
   converting the message to the correct Message<T> type. */
template <typename T>
std::function<void(const BaseMessage&)> wrap(void (*handler)(const Message<T>&))
{
  return [=](const BaseMessage& msg) {
    handler(static_cast<const Message<T>&>(msg));
  };
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
    {TMessage::NETWORK_SEND, wrap(handler1)},
    {TMessage::WHATEVER, wrap(handler2)}
  };

  std::thread t1(sender1, std::ref(Q));
  std::thread t2(sender2, std::ref(Q));

  while (true) {
    while (!Q.empty()) {
      std::shared_ptr<const BaseMessage> msg = Q.pop();
      handlers[msg->type](*msg);
    }
  }
}
