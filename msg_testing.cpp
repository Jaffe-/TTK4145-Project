#include <iostream>
#include <map>
#include <algorithm>
#include <functional>
#include <string>

enum class TMessage {
  NETWORK_SEND, WHATEVER, DRIVER_FLOORS
};

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
auto wrap(void (*handler)(Message<T>))
{
  return [=](IMessage* msg) {
    handler(*static_cast<Message<T>*>(msg));
  };
}

void handler1(Message<std::string> s)
{
  std::cout << "STRING: " << s.data << std::endl;
}

void handler2(Message<int> s)
{
  std::cout << "INT: " << s.data << std::endl;
}

int main()
{
  std::map<TMessage, std::function<void(IMessage*)>> handlers = {
    {TMessage::NETWORK_SEND, wrap(handler1)},
    {TMessage::WHATEVER, wrap(handler2)}
  };

  Message<std::string> m1(TMessage::NETWORK_SEND, "HEI HEI");
  Message<int> m2(TMessage::WHATEVER, 1234);
  Message<std::string> m3(TMessage::NETWORK_SEND, "hoho");


  handlers[TMessage::NETWORK_SEND](&m1);
  handlers[TMessage::WHATEVER](&m2);
  handlers[TMessage::NETWORK_SEND](&m3);
}
