#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <typeinfo>
#include "event.hpp"


/*
  Thread safe event queue

  Pushing to the queue is done element by element using the push function.

  The reader calls handle_events() with three things:

    - A supplied lock: The reader can choose to immediately acquire a lock on
      the queue using acquire(), or sleep until a new event occurs using wait().

    - The instance of the class (module) which is to handle the event.

    - An event list which contains the events that the module will respond to.
      For each event in the list, the class needs to implement a function called
      notify, which takes such an event type as argument. This is enforced at
      compile time.
*/

class EventQueue {
public:
  using queue_t = std::deque<std::unique_ptr<Event>>;

  /* Acquire immediately locks the queue */
  std::unique_lock<std::mutex> acquire();

  /* Wait will suspend the thread until a new event has arrived */
  std::unique_lock<std::mutex> wait();

  /* Push any given event into the queue. The event object must be copy
     constructible and derive from Event */
  template <typename T>
  void push(const T& msg) {
    {
      auto lock = acquire();
      queue.push_back(std::make_unique<T>(msg));
    }
    new_event.notify_one();
  }

  /* For each event in the queue, Class::notify is called on the given Class
     instance. The EventList given contains the event types that will be
     handled */
  template <typename Class, typename... Events>
  void handle_events(std::unique_lock<std::mutex> lock, Class* instance, EventList<Events...> events) {
    for (auto& event_ptr : take_events(std::move(lock))) {
      call_notify(*event_ptr, instance, events);
    }
  }

private:

  /* Use the given lock to take all current event (move them out of the queue) */
  queue_t take_events(std::unique_lock<std::mutex> lock);

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
