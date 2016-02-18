#include <iostream>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <memory>
#include "message_queue.hpp"

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
  //  std::queue<std::shared_ptr<BaseMessage>> Q;

  std::map<TMessage, std::function<void(const BaseMessage&)>> handlers = {
    {TMessage::NETWORK_SEND, wrap(handler1)},
    {TMessage::WHATEVER, wrap(handler2)}
  };

  Q.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, "HEI HEI"));
  Q.push(std::make_shared<Message<int>>(TMessage::WHATEVER, 1234));
  Q.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, "hoho"));
  Q.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, "hals"));

  while (!Q.empty()) {
    std::shared_ptr<BaseMessage> msg = Q.pop();
    handlers[msg->type](*msg);
  }
}
