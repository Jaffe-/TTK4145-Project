#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "serialization.hpp"
#include <typeinfo>

template <typename T>
class Message;

template <typename T>
class SerializableMessage;

/* This should contain all fields common to all kinds of messages. */
class BaseMessage {
public:
  /*
    This operator allows a BaseMessage object to be automatically converted
    to a Message<T>, where T depends on the context - i.e. when left hand side
    in an assignment is a specific Message<T> or when a BaseMessage reference/
    pointer is given to a function taking a specific Message<T>.
  */
  template <typename T>
  operator const Message<T>&() const {
    if (typeid(T) == get_type())
      return static_cast<const Message<T>&>(*this);
    else throw std::bad_cast();
  }

  /* Virtualizing the Serializable& conversion operator allows one to serialize
     a message without knowing the type of the data in it. */
  virtual operator const Serializable&() const {
    throw std::bad_cast();
  };

  /* Each message will return type_info about the actual type they're
     carrying */
  virtual const std::type_info& get_type() const = 0;
};

/* The concrete class for messages, where data is of type T */
template <typename T>
class Message : public BaseMessage {
public:
  Message(const T& data) : data(data) {};
  const T data;

  const std::type_info& get_type() const override {
    return typeid(T);
  }
};

template <typename T>
class SerializableMessage : public Message<T>, public Serializable {
public:
  operator const Serializable&() const override {
    return static_cast<const Serializable&>(*this);
  }

  explicit SerializableMessage(const T& data) : Message<T>(data) {};

  /* Construct from JSON object */
  explicit SerializableMessage(const json& js)
    : Message<T>(T(js["data"])) {};

  /* Construct from JSON string */
  SerializableMessage(const std::string& json_string)
    : SerializableMessage(json::parse(json_string)) {};

  json get_json() const override {
    return {
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
