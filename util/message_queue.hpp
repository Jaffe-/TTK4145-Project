#pragma once
#include <memory>
#include <queue>

enum class TMessage {
  NETWORK_SEND, WHATEVER, DRIVER_FLOORS
};

/* This should contain all fields common to all kinds of messages. */
class BaseMessage {
public:
  BaseMessage(TMessage type) : type(type) {};
  TMessage type;
};

/* The concrete class for messages, where data is of type T */
template <typename T>
class Message : public BaseMessage {
public:
  Message(TMessage type, const T& data) : data(data), BaseMessage(type) {};
  T data;
};

class MessageQueue {
public:
  void push(const std::shared_ptr<BaseMessage>& msg);
  std::shared_ptr<BaseMessage> pop();
  bool empty() const {
    return queue.empty();
  };
private:
  std::queue<std::shared_ptr<BaseMessage>> queue;
};
