#include "logic.hpp"
#include "../util/logger.hpp"
#include "../driver/driver_events.hpp"
#include "../network/network_events.hpp"
#include "fsm.hpp"
#include <climits>

Logic::Logic(bool use_simulator, const std::string& port)
  : driver(event_queue, use_simulator),
    network(event_queue, port),
    driver_thread([&] { driver.run(); }),
    network_thread([&] { network.run(); })
{
  LOG_INFO("Logic module started");

  event_queue.add_handler<ExternalButtonEvent>(this, &Logic::notify);
  event_queue.add_handler<StateUpdateEvent>(this, &Logic::notify);
  event_queue.add_handler<NetworkReceiveEvent<StateUpdateEvent>>(this, &Logic::notify);
  event_queue.add_handler<NetworkReceiveEvent<ExternalButtonEvent>>(this, &Logic::notify);
}

void Logic::choose_elevator(Button button)
{
  int min = INT_MAX;
  std::string min_id;
  for (const auto& pair : elevator_states) {
    const State& state = pair.second;
    auto fsm = SimulatedFSM(state);
    int steps = fsm.calculate(button);
    if (steps < min) {
      min = steps;
      min_id = pair.first;
    }
  }

  assert(min != INT_MAX);

  if (min_id == "me") {
    driver.event_queue.push(OrderUpdateEvent(button_floor(button),
					     button_type(button)));
  }
}

void Logic::notify(const ExternalButtonEvent& event)
{
  network.event_queue.push(event);
  choose_elevator(event.button);
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
  choose_elevator(event.data.button);
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
