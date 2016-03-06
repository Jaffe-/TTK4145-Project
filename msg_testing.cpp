#include <iostream>
#include <map>
#include <functional>
#include <string>
#include <memory>
#include "util/message_queue.hpp"
#include <thread>
#include <chrono>
#include <cassert>
#include <typeindex>
#include <unordered_map>
#include "util/serialization.hpp"

class Dummy : public Serializable {
public:
  std::string s;

  Dummy(const std::string& s) : s(s) {};

  Dummy(const json& js) : s((std::string&)js["dummy"]) {};

  json get_json() const override {
    return {{"dummy", s}}; 
  }
};

void sender1(MessageQueue& queue)
{
  unsigned int id = 0;
  while (true) {
    for (int i = 0; i < 5; i++) {
      std::string s = "1::Message number " + std::to_string(id++);
      Dummy d = s;
      auto msg = std::make_shared<SerializableMessage<Dummy>>(d);
      queue.push(std::move(msg));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

void sender2(MessageQueue& queue)
{
  unsigned int id = 0;
  while(true) {
    queue.push(std::make_shared<Message<int>>(id++));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
}

void serializer(const Serializable& s)
{
  std::cout << s.serialize() << std::endl;
}

void handler1(const Message<Dummy>& s)
{
  std::cout << "STRING: " << s.data.s << std::endl;
}

void handler2(const Message<int>& s)
{
  std::cout << "INT: " << s.data << std::endl;
}

int main()
{
  MessageQueue Q;

  const std::unordered_map<std::type_index, std::function<void(const BaseMessage&)>> handlers = {
    {typeid(Dummy), handler1},
    {typeid(int), handler2}
  };

  std::thread t1(sender1, std::ref(Q));
  std::thread t2(sender2, std::ref(Q));

  /* Sleeping version */
  while (true) {
    for (const auto& msg : Q.take_messages(Q.wait())) {
      if (msg->serializable()) {
	serializer(*msg);
      }
      if (handlers.find(msg->get_type()) != handlers.end())
	handlers.at(msg->get_type())(*msg);
      else
	std::cout << "unhandled" << std::endl;
    }
  }
  /*
  while (true) {
    for (const auto& msg : Q.take_messages(Q.acquire())) {
      handlers[msg->type](*msg);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }*/
     
}
