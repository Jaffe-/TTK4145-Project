#include "logic.hpp"
#include "../util/logger.hpp"

Logic::Logic(bool use_simulator, const std::string& port) : driver(message_queue, use_simulator),
							    network(message_queue, port),
							    driver_thread([&] { driver.run(); }),
							    network_thread([&] { network.run(); })
{
  LOG_INFO("Logic module started");

}

void Logic::run()
{
  while (true);
}
