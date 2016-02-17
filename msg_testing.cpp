#include <iostream>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <memory>
#include <queue>

enum class TMessage {
  NETWORK_SEND, WHATEVER, DRIVER_FLOORS
};

/* This should contain all fields common to all kinds of messages. */
class IMessage {
public:
  IMessage(TMessage type) : type(type) {};
  TMessage type;
};

template <typename T>
class Message : public IMessage {
public:
  Message(TMessage type, const T& data) : data(data), IMessage(type) {};
  T data;
};

/* Wraps a handler function taking Message<T> in a lambda which takes care of
   converting the message to the correct Message<T> type. */
template <typename T>
std::function<void(const IMessage&)> wrap(void (*handler)(const Message<T>&))
{
  return [=](const IMessage& msg) {
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
  std::queue<std::shared_ptr<IMessage>> Q;

  std::map<TMessage, std::function<void(const IMessage&)>> handlers = {
    {TMessage::NETWORK_SEND, wrap(handler1)},
    {TMessage::WHATEVER, wrap(handler2)}
  };

  Q.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, "HEI HEI"));
  Q.push(std::make_shared<Message<int>>(TMessage::WHATEVER, 1234));
  Q.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, "hoho"));
  Q.push(std::make_shared<Message<std::string>>(TMessage::NETWORK_SEND, "hals"));

  while (!Q.empty()) {
    std::shared_ptr<IMessage> msg {std::move(Q.front())};
    handlers[msg->type](*msg);
    Q.pop();
  }
}
