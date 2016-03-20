#include "logic.hpp"
#include "../util/logger.hpp"
#include "../driver/driver_events.hpp"
#include "../network/network_events.hpp"

Logic::Logic(bool use_simulator, const std::string& port)
  : driver(event_queue, use_simulator),
    network(event_queue, port),
    driver_thread([&] { driver.run(); }),
    network_thread([&] { network.run(); })
{
  LOG_INFO("Logic module started");

  event_queue.add_handler<ExternalButtonEvent>([this]
						 (const ExternalButtonEvent& e) {
						   driver.event_queue.push(OrderUpdateEvent(button_floor(e.button),
											    button_type(e.button)));

						 });

  event_queue.add_handler<StateUpdateEvent>([this]
					    (const StateUpdateEvent& e) {
					      network.event_queue.push(e);
					    });

  event_queue.add_handler<NetworkReceiveStateEvent>([this]
						     (const NetworkReceiveStateEvent& e) {
						       LOG_DEBUG("Received from " << e.ip << ": " << e.update_event);
						     });
}

void Logic::run()
{
  while (true) {
    event_queue.handle_events(event_queue.wait());
  };
}
