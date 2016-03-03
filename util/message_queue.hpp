#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>

enum class TMessage {
  NETWORK_SEND, WHATEVER, DRIVER_FLOORS
};


template <typename T>
class Message;

/* This should contain all fields common to all kinds of messages. */
class BaseMessage {
public:
  BaseMessage(TMessage type) : type(type) {};
  TMessage type;

  template <typename T>
  operator const Message<T>&() const {
    return static_cast<const Message<T>&>(*this);
  }
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
  using queue_t = std::deque<std::shared_ptr<const BaseMessage>>;

  std::unique_lock<std::mutex> wait();
  std::unique_lock<std::mutex> acquire();

  void push(const std::shared_ptr<BaseMessage>& msg);

  queue_t take_messages(std::unique_lock<std::mutex> lock);

private:
  queue_t queue;
  std::mutex mut;
  std::condition_variable new_message;
};
