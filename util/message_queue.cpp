#include "message_queue.hpp"
#include <thread>
#include <cassert>


void MessageQueue::push(const std::unique_lock<std::mutex>& lock, const std::shared_ptr<BaseMessage>& msg)
{
  assert(lock);
  queue.push(std::move(msg));
  cv.notify_one();
}

void MessageQueue::push(const std::shared_ptr<BaseMessage>& msg)
{
  push(acquire(), msg);
}

/* Acquire immediately locks the queue */
std::unique_lock<std::mutex> MessageQueue::acquire()
{
  std::unique_lock<std::mutex> lock(mut);
  return lock;
}

/* Wait will suspend the thread until the queue has been altered */
std::unique_lock<std::mutex> MessageQueue::wait()
{
  std::unique_lock<std::mutex> lock(mut);
  cv.wait(lock, [this] {return !queue.empty(); });
  return lock;
}

std::shared_ptr<const BaseMessage> MessageQueue::pop(const std::unique_lock<std::mutex>& lock)
{
  assert(lock);
  auto msg = std::move(queue.front());
  queue.pop();
  return msg;  
}

std::shared_ptr<const BaseMessage> MessageQueue::pop()
{
  return pop(acquire());
}

bool MessageQueue::empty(const std::unique_lock<std::mutex>& lock)
{
  assert(lock);
  return queue.empty();
};

bool MessageQueue::empty()
{
  return empty(acquire());
}
