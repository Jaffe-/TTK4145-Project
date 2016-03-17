#include "logic.hpp"
#include "../util/logger.hpp"
#include "../driver/driver_events.hpp"

Logic::Logic(bool use_simulator, const std::string& port) : driver(message_queue, use_simulator),
							    network(message_queue, port),
							    driver_thread([&] { driver.run(); }),
							    network_thread([&] { network.run(); })
{
  LOG_INFO("Logic module started");

  message_queue.add_handler<ExternalButtonEvent>([this]
						 (const ExternalButtonEvent& e) {
						   driver.message_queue.push(OrderUpdateEvent(button_floor(e.button),
											      button_type(e.button)));

						 });

}

void Logic::run()
{
  while (true) {
    message_queue.handle_messages(message_queue.wait());
  };
}
