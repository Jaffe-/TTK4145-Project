#include "message_queue.hpp"
#include <thread>
#include <cassert>


std::unique_lock<std::mutex> MessageQueue::acquire()
{
  return std::unique_lock<std::mutex>(mut);
}

std::unique_lock<std::mutex> MessageQueue::wait()
{
  std::unique_lock<std::mutex> lock(mut);
  new_message.wait(lock, [this] {return !queue.empty(); });
  return lock;
}

MessageQueue::queue_t MessageQueue::take_messages(std::unique_lock<std::mutex> lock)
{
  assert(lock && lock.mutex() == &mut);
  auto messages = std::move(queue);
  queue = queue_t();
  return messages;
}

void MessageQueue::handle_messages(const queue_t& queue)
{
  for (const auto& msg : queue) {
    if (handlers.find(typeid(*msg)) != handlers.end()) {
      handlers[typeid(*msg)](*msg);
    }
  }
}
