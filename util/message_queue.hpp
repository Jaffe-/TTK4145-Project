#pragma once
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

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
  std::unique_lock<std::mutex> wait();
  std::unique_lock<std::mutex> acquire();

  void push(const std::unique_lock<std::mutex>& lock, const std::shared_ptr<BaseMessage>& msg);
  void push(const std::shared_ptr<BaseMessage>& msg);
  std::shared_ptr<const BaseMessage> pop(const std::unique_lock<std::mutex>& lock);
  std::shared_ptr<const BaseMessage> pop();
  bool empty(const std::unique_lock<std::mutex>& lock);
  bool empty();

private:
  //  std::shared_ptr<BaseMessage> do_pop();
  
  std::queue<std::shared_ptr<const BaseMessage>> queue;
  std::mutex mut;
  std::condition_variable cv;
};
