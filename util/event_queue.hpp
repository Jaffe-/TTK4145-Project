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


class Event {
public:
  virtual bool serializable() const {
    return false;
  };

  /* Used to cast an event to its concrete derived type */
  template <typename T>
  operator const T&() const {
    if (typeid(T) == typeid(*this))
      return static_cast<const T&>(*this);
    else throw std::bad_cast();
  }

  /* Used to cast itself to Serializable. Must be dynamic cast since
     Event doesn't itself derive from Serializable. */
  operator const Serializable&() const {
    return dynamic_cast<const Serializable&>(*this);
  };
};


class EventQueue {
public:
  using queue_t = std::deque<std::shared_ptr<const Event>>;
  using map_t = std::unordered_map<std::type_index, std::function<void(const Event&)>>;

  /* Acquire immediately locks the queue */
  std::unique_lock<std::mutex> wait();

  /* Wait will suspend the thread until a new message has arrived */
  std::unique_lock<std::mutex> acquire();

  /* Push any given message into the queue. The message object must be copy
     constructible and derive from Event */
  template <typename T>
  void push(const T& msg) {
    {
      auto lock = acquire();
      queue.push_back(std::make_shared<T>(msg));
    }
    new_event.notify_one();
  }

  /* Use the given lock to take all current messages (move them out of the queue) */
  queue_t take_events(std::unique_lock<std::mutex> lock);


  /* Add a message handler function */
  template <typename T>
  void add_handler(std::function<void(const T&)> handler) {
    handlers[typeid(T)] = handler;
  }

  /* Add a member function of a class as a message handler */
  template <typename T, typename Class>
  void add_handler(Class* instance, void(Class::*f)(const T&)) {
    add_handler<T>([instance, f] (const T& m) { (*instance.*f)(m); });
  }

  /* Call the right message handlers for the messages in the given queue */
  void handle_events(const queue_t& queue);

  /* A practical overload */
  void handle_events(std::unique_lock<std::mutex> lock) {
    handle_events(take_events(std::move(lock)));
  }

private:
  queue_t queue;
  std::mutex mut;
  std::condition_variable new_event;

  map_t handlers;
};