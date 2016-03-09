#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "serialization.hpp"
#include <typeinfo>

/* This should contain all fields common to all kinds of messages. */
class Message {
public:
  virtual bool serializable() const {
    return false;
  };

  /*
    This operator allows a BaseMessage object to be automatically converted
    to a Message<T>, where T depends on the context - i.e. when left hand side
    in an assignment is a specific Message<T> or when a BaseMessage reference/
    pointer is given to a function taking a specific Message<T>.
  */
  template <typename T>
  operator const T&() const {
    if (typeid(T) == typeid(*this))
      return static_cast<const T&>(*this);
    else throw std::bad_cast();
  }

  /* Virtualizing the Serializable& conversion operator allows one to serialize
     a message without knowing the type of the data in it. */
  operator const Serializable&() const {
    return dynamic_cast<const Serializable&>(*this);
  };
};

class MessageQueue {
public:
  using queue_t = std::deque<std::shared_ptr<const Message>>;

  std::unique_lock<std::mutex> wait();
  std::unique_lock<std::mutex> acquire();

  void push(const std::shared_ptr<Message>& msg);

  queue_t take_messages(std::unique_lock<std::mutex> lock);

private:
  queue_t queue;
  std::mutex mut;
  std::condition_variable new_message;
};
