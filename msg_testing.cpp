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

/* Helpful in C++11 */
template <typename T>
std::unique_ptr<T> make_unique_ptr(T* ptr)
{
  return std::unique_ptr<T>(ptr);
}

int main()
{
  std::queue<std::unique_ptr<IMessage>> Q;

  std::map<TMessage, std::function<void(const IMessage&)>> handlers = {
    {TMessage::NETWORK_SEND, wrap(handler1)},
    {TMessage::WHATEVER, wrap(handler2)}
  };

  /* C++14:
  Q.push(std::make_unique<Message<std::string>>(TMessage::NETWORK_SEND, "HEI HEI"));
  Q.push(std::make_unique<Message<int>>(TMessage::WHATEVER, 1234));
  Q.push(std::make_unique<Message<std::string>>(TMessage::NETWORK_SEND, "hoho"));
  Q.push(std::make_unique<Message<std::string>>(TMessage::NETWORK_SEND, "hals"));
  */

  Q.push(make_unique_ptr(new Message<std::string>(TMessage::NETWORK_SEND, "HEI HEI")));
  Q.push(make_unique_ptr(new Message<int>(TMessage::WHATEVER, 1234)));
  Q.push(make_unique_ptr(new Message<std::string>(TMessage::NETWORK_SEND, "hoho")));
  Q.push(make_unique_ptr(new Message<std::string>(TMessage::NETWORK_SEND, "hals")));

  while (!Q.empty()) {
    std::unique_ptr<IMessage> msg {std::move(Q.front())};
    handlers[msg->type](*msg);
    Q.pop();
  }
}
