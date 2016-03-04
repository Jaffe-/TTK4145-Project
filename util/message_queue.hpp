#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "serialization.hpp"

enum class TMessage {
  NETWORK_SEND, WHATEVER, DRIVER_FLOORS
};


template <typename T>
class Message;

template <typename T>
class SerializableMessage;

/* This should contain all fields common to all kinds of messages. */
class BaseMessage {
public:
  BaseMessage(TMessage type) : type(type) {};
  TMessage type;

  /*
    This operator allows a BaseMessage object to be automatically converted
    to a Message<T>, where T depends on the context - i.e. when left hand side
    in an assignment is a specific Message<T> or when a BaseMessage reference/
    pointer is given to a function taking a specific Message<T>.

    Warning: This is not type safe.
  */
  template <typename T>
  operator const Message<T>&() const {
    return static_cast<const Message<T>&>(*this);
  }

  template <typename T>
  operator const SerializableMessage<T>&() const {
    return static_cast<const SerializableMessage<T>&>(*this);
  }
};

/* The concrete class for messages, where data is of type T */
template <typename T>
class Message : public BaseMessage {
public:
  Message(TMessage type, const T& data) : data(data), BaseMessage(type) {};
  const T data;
};

template <typename T>
class SerializableMessage : public Message<T>, public Serializable {
public:
  SerializableMessage(TMessage type, const T& data) : Message<T>(type, data) {};

  /* Construct from JSON object */
  SerializableMessage(const json& js)
    : Message<T>((TMessage)((int)js["type"]), js["data"]) {};

  /* Construct from JSON string */
  SerializableMessage(const std::string& json_string)
    : SerializableMessage(json::parse(json_string)) {};

  json get_json() const {
    return {
      {"type", static_cast<int>(this->type)},
      {"data", this->data.get_json()}
    };
  }
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
