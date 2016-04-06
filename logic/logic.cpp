#include "logic.hpp"
#include "../util/logger.hpp"
#include "../driver/driver_events.hpp"
#include "../network/network_events.hpp"
#include "fsm.hpp"

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
						   network.event_queue.push(e);
						   SimulatedFSM sim(elevator_states["me"]);
						   LOG_DEBUG("This is estimated to take "
							     << sim.calculate(e.button)
							     << " steps.");
						 });

  event_queue.add_handler<StateUpdateEvent>(this, &Logic::notify);
  event_queue.add_handler<NetworkReceiveEvent<StateUpdateEvent>>(this, &Logic::notify);
  event_queue.add_handler<NetworkReceiveEvent<ExternalButtonEvent>>(this, &Logic::notify);
}

void Logic::notify(const StateUpdateEvent& event)
{
  elevator_states["me"] = event.state;
  network.event_queue.push(event);
}

void Logic::notify(const NetworkReceiveEvent<StateUpdateEvent>& event)
{
  LOG_DEBUG("Received " << event);
  elevator_states[event.ip] = event.data.state;
}

void Logic::notify(const NetworkReceiveEvent<ExternalButtonEvent>& event)
{
  LOG_DEBUG("Received " << event);
}

void Logic::notify(const LostConnectionEvent& event)
{
  auto it = elevator_states.find(event.ip);
  if (it != elevator_states.end())
    elevator_states.erase(it);
}

void Logic::run()
{
  while (true) {
    event_queue.handle_events(event_queue.wait());
  };
}
