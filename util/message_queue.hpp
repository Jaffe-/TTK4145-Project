#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "serialization.hpp"
#include <typeinfo>
#include <typeindex>
#include <unordered_map>


class Message {
public:
  virtual bool serializable() const {
    return false;
  };

  /* Used to cast a Message to its concrete derived type */
  template <typename T>
  operator const T&() const {
    if (typeid(T) == typeid(*this))
      return static_cast<const T&>(*this);
    else throw std::bad_cast();
  }

  /* Used to cast itself to Serializable. Must be dynamic cast since
     Message doesn't itself derive from Serializable. */
  operator const Serializable&() const {
    return dynamic_cast<const Serializable&>(*this);
  };
};


class MessageQueue {
public:
  using queue_t = std::deque<std::shared_ptr<const Message>>;
  using map_t = std::unordered_map<std::type_index, std::function<void(const Message&)>>;

  /* Acquire immediately locks the queue */
  std::unique_lock<std::mutex> wait();

  /* Wait will suspend the thread until a new message has arrived */
  std::unique_lock<std::mutex> acquire();

  /* Push any given message into the queue. The message object must be copy
     constructible and derive from Message */
  template <typename T>
  void push(const T& msg) {
    {
      auto lock = acquire();
      queue.push_back(std::make_shared<T>(msg));
    }
    new_message.notify_one();
  }

  /* Use the given lock to take all current messages (move them out of the queue) */
  queue_t take_messages(std::unique_lock<std::mutex> lock);


  /* Add message handler function */
  template <typename T>
  void add_handler(std::function<void(const T&)> handler) {
    handlers[typeid(T)] = handler;
  }

  /* Call the right message handlers for the messages in the given queue */
  void handle_messages(const queue_t& queue);

  /* A practical overload */
  void handle_messages(std::unique_lock<std::mutex> lock) {
    handle_messages(take_messages(std::move(lock)));
  }

private:
  queue_t queue;
  std::mutex mut;
  std::condition_variable new_message;

  map_t handlers;
};
