#include "event_queue.hpp"
#include <thread>
#include <cassert>


std::unique_lock<std::mutex> EventQueue::acquire()
{
  return std::unique_lock<std::mutex>(mut);
}

std::unique_lock<std::mutex> EventQueue::wait()
{
  std::unique_lock<std::mutex> lock(mut);
  new_event.wait(lock, [this] {return !queue.empty(); });
  return lock;
}

EventQueue::queue_t EventQueue::take_events(std::unique_lock<std::mutex> lock)
{
  assert(lock && lock.mutex() == &mut);
  auto events = std::move(queue);
  queue = queue_t();
  return events;
}

void EventQueue::handle_events(const queue_t& queue)
{
  for (const auto& event : queue) {
    if (handlers.find(typeid(*event)) != handlers.end()) {
      handlers[typeid(*event)](*event);
    }
  }
}
