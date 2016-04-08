#include "logic.hpp"
#include "../util/logger.hpp"
#include "../driver/driver_events.hpp"
#include "../network/network_events.hpp"
#include "fsm.hpp"
#include <climits>
#include <string>

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
  event_queue.add_handler<NewConnectionEvent>(this, &Logic::notify);
  event_queue.add_handler<LostConnectionEvent>(this, &Logic::notify);
  event_queue.add_handler<LostNetworkEvent>(this, &Logic::notify);
}

/* Calculate the cost function (using the simulated FSM) for each elevator and
   determine if this elevator should take the given order. If we have the
   minimum value, we should take the order even though others may have it. */
void Logic::choose_elevator(Button button)
{
  int min = INT_MAX;
  int our_min = INT_MAX;
  for (const auto& pair : elevator_states) {
    const State& state = pair.second;
    int steps = SimulatedFSM(state).calculate(button);
    LOG_DEBUG("Calculated steps " << steps << " for id " << pair.first);
    if (steps < min) {
      min = steps;
    }
    if (pair.first == "me") {
      our_min = steps;
    }
  }

  assert(min != INT_MAX);

  if (min == our_min) {
    driver.event_queue.push(OrderUpdateEvent(button_floor(button),
					     button_type(button)));
  }
}

/* When an external button event occurs, it is broadcasted to the other
   connections, and the choose function is run to determine whether this
   elevator should take the order */
void Logic::notify(const ExternalButtonEvent& event)
{
  network.event_queue.push(event);
  choose_elevator(event.button);
}

/* When a state update event occurs, our own state in elevator_states should be
   updated, and it should be broadcasted to the other elevators. */
void Logic::notify(const StateUpdateEvent& event)
{
  elevator_states["me"] = event.state;
  network.event_queue.push(event);
}

/* When a state update event is received from the network, it should be stored
   in elevator_states, creating a new index if needed. */
void Logic::notify(const NetworkReceiveEvent<StateUpdateEvent>& event)
{
  LOG_DEBUG("Received " << event);
  elevator_states[event.ip] = event.data.state;
}

/* When an external button event is received from the network, the choose
   function is run to determine whether this elevator should take the order. */
void Logic::notify(const NetworkReceiveEvent<ExternalButtonEvent>& event)
{
  LOG_DEBUG("Received " << event);
  choose_elevator(event.data.button);
}

/* When a connection is lost, that elevator's state should be removed from
   elevator_states, since it should no longer be part of the decision making */
void Logic::notify(const LostConnectionEvent& event)
{
  auto it = elevator_states.find(event.ip);
  if (it != elevator_states.end())
    elevator_states.erase(it);
}

/* When a new elevator appears we must send our latest state to it. */
void Logic::notify(const NewConnectionEvent&)
{
  StateUpdateEvent state(elevator_states["me"]);
  network.event_queue.push(state);
}

/* When our own network connection is lost, remove all elevator states
   except our own, as the information about the rest of the elevators
   will be outdated. */
void Logic::notify(const LostNetworkEvent&)
{
  for (auto it = elevator_states.begin(); it != elevator_states.end(); ) {
    if (it->first != "me")
      it = elevator_states.erase(it);
    else
      it++;
  }
}

void Logic::run()
{
  while (true) {
    event_queue.handle_events(event_queue.wait());
  };
}
