#include "message_queue.hpp"
#include <thread>
#include <cassert>


/* Acquire immediately locks the queue */
std::unique_lock<std::mutex> MessageQueue::acquire()
{
  return std::unique_lock<std::mutex>(mut);
}

/* Wait will suspend the thread until a new message has arrived */
std::unique_lock<std::mutex> MessageQueue::wait()
{
  std::unique_lock<std::mutex> lock(mut);
  new_message.wait(lock, [this] {return !queue.empty(); });
  return lock;
}


/* Push uses acquire() to get the lock and moves its argument into the queue */
void MessageQueue::push(const std::shared_ptr<Message>& msg)
{
  {
    auto lock = acquire();
    queue.push_back(std::move(msg));
  }
  new_message.notify_one();
}


/* Use the given lock to take all current messages (move them out of the queue) */ 
MessageQueue::queue_t MessageQueue::take_messages(std::unique_lock<std::mutex> lock)
{
  assert(lock && lock.mutex() == &mut);
  auto messages = std::move(queue);
  queue = queue_t();
  return messages;
}
