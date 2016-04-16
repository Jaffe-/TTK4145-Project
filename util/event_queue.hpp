#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <typeinfo>
#include "event.hpp"

class EventQueue {
public:
  using queue_t = std::deque<std::shared_ptr<const Event>>;

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

  /* For each event in the queue, Class::notify is called on the given Class
     instance. The EventList given contains the event types that will be
     handled */
  template <typename Class, typename... Events>
  void handle_events(std::unique_lock<std::mutex> lock, Class* instance, EventList<Events...> events)
  {
    for (const auto& event : take_events(std::move(lock))) {
      call_notify(*event, instance, events);
    }
  }

private:

  /* This is a recursive template which will expand into a chain of comparisons
     of the event's typeid to each type in the event list, issuing a call to
     Class::notify on the given instance if they match. */
  template <typename Class, typename EventType, typename... Rest>
  void call_notify(const Event& event, Class* instance, EventList<EventType, Rest...>) {
    if (typeid(event) == typeid(EventType))
      instance->notify((const EventType&)event);
    else
      call_notify(event, instance, EventList<Rest...>{});
  }

  /* Base case for the above function. This is reached if none of the events in
     the EventList matched the given event. */
  template <typename Class>
  void call_notify(const Event&, Class*, EventList<>) { }

  queue_t queue;
  std::mutex mut;
  std::condition_variable new_event;
};
