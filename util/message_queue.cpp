#include "message_queue.hpp"
#include <thread>
#include <cassert>


/* Acquire immediately locks the queue */
std::unique_lock<std::mutex> MessageQueue::acquire()
{
  std::unique_lock<std::mutex> lock(mut);
  return lock;
}

/* Wait will suspend the thread until a new message has arrived */
std::unique_lock<std::mutex> MessageQueue::wait()
{
  std::unique_lock<std::mutex> lock(mut);
  new_message.wait(lock, [this] {return !queue.empty(); });
  return lock;
}


void MessageQueue::push(const std::unique_lock<std::mutex>& lock, const std::shared_ptr<BaseMessage>& msg)
{
  assert(lock && lock.mutex() == &mut);
  queue.push(std::move(msg));
  new_message.notify_one();
}

void MessageQueue::push(const std::shared_ptr<BaseMessage>& msg)
{
  push(acquire(), msg);
}



std::shared_ptr<const BaseMessage> MessageQueue::pop(const std::unique_lock<std::mutex>& lock)
{
  assert(lock && lock.mutex() == &mut);
  auto msg = std::move(queue.front());
  queue.pop();
  return msg;  
}

std::shared_ptr<const BaseMessage> MessageQueue::pop()
{
  return pop(acquire());
}


bool MessageQueue::empty(const std::unique_lock<std::mutex>& lock) const
{
  assert(lock && lock.mutex() == &mut);
  return queue.empty();
};

bool MessageQueue::empty()
{
  return empty(acquire());
}
