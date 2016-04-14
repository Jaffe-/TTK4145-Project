#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include "event.hpp"

class EventQueue {
public:
  using queue_t = std::deque<std::shared_ptr<const Event>>;
  using map_t = std::unordered_map<std::type_index, std::function<void(const Event&)>>;

  /* Acquire immediately locks the queue */
  std::unique_lock<std::mutex> wait();

  /* Wait will suspend the thread until a new event has arrived */
  std::unique_lock<std::mutex> acquire();

  /* Push any given event into the queue. The message object must be copy
     constructible and derive from Event */
  template <typename T>
  void push(const T& msg) {
    {
      auto lock = acquire();
      queue.push_back(std::make_shared<T>(msg));
    }
    new_event.notify_one();
  }

  /* Use the given lock to take all current event (move them out of the queue) */
  queue_t take_events(std::unique_lock<std::mutex> lock);

  /* Take a list of event types which the event queue should respond to, and a
     pointer to an object which has notify functions for each event type. */
  template <typename Class, typename EventType, typename... Rest>
  void call_notify(const Event& event, Class* instance, EventList<EventType, Rest...>) {
    if (typeid(event) == typeid(EventType))
      instance->notify((const EventType&)event);
    else
      call_notify(event, instance, EventList<Rest...>{});
  }

  /* Base case for the above function */
  template <typename Class>
  void call_notify(const Event&, Class*, EventList<>) { }

  template <typename Class, typename... Events>
  void handle_events(std::unique_lock<std::mutex> lock, Class* instance, EventList<Events...> events)
  {
    for (const auto& event : take_events(std::move(lock))) {
      call_notify(*event, instance, events);
    }
  }

private:
  queue_t queue;
  std::mutex mut;
  std::condition_variable new_event;

  map_t handlers;
};
