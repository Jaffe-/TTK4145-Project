#include "message_queue.hpp"

void MessageQueue::push(const std::shared_ptr<BaseMessage>& msg)
{
  // LOCK
  queue.push(std::move(msg));
  // UNLOCK
}

std::shared_ptr<BaseMessage> MessageQueue::pop()
{
  // LOCK
  auto msg = std::move(queue.front());
  queue.pop();
  return msg;
  // UNLOCK
}
