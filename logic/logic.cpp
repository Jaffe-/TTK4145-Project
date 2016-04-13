#include "logic.hpp"
#include "events.hpp"
#include "../util/logger.hpp"
#include "../driver/events.hpp"
#include "../network/events.hpp"
#include "simulatedfsm.hpp"
#include <climits>
#include <string>
#include <fstream>

Logic::Logic(bool use_simulator, const std::string& port)
  : driver(event_queue, use_simulator),
    network(event_queue, port),
    driver_thread([&] { driver.run(); }),
    network_thread([&] { network.run(); })
{
  LOG_INFO("Logic module started");

  event_queue.listen(this, EventList<ExternalButtonEvent,
		                     StateUpdateEvent,
		                     NetworkMessageEvent<StateUpdateEvent>,
		                     NetworkMessageEvent<ExternalButtonEvent>,
		                     NewConnectionEvent,
		                     LostConnectionEvent,
		     LostNetworkEvent,
		     NetworkMessageEvent<StateUpdateReqEvent>>());

  std::vector<std::vector<bool>> orders = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};
  if (restore_orders(orders)) {
    LOG_INFO("Restoring previous orders");
    for (int floor = 0; floor < FLOORS; floor++) {
      for (int type = 0; type <= 2; type++) {
	if (orders[floor][type])
	  driver.event_queue.push(OrderUpdateEvent(floor, type));
      }
    }
  }
}

/* Calculate the cost function (using the simulated FSM) for each elevator and
   determine if this elevator should take the given order. If we have the
   minimum value, we should take the order even though others may have it. */
void Logic::choose_elevator(int floor, ButtonType type)
{
  int min = INT_MAX;
  int our_min = INT_MAX;
  for (const auto& pair : elevator_infos) {
    const ElevatorInfo& elevator_info = pair.second;
    if (elevator_info.active) {
      int steps = SimulatedFSM(elevator_info.state).calculate(floor, static_cast<int>(type));
      LOG_DEBUG("Calculated steps " << steps << " for id " << pair.first);
      if (steps < min) {
	min = steps;
      }
      if (pair.first == "me") {
	our_min = steps;
      }
    }
  }

  assert(min != INT_MAX);

  if (min == our_min) {
    driver.event_queue.push(OrderUpdateEvent(floor, static_cast<int>(type)));
  }
}

/* When an external button event occurs, it is broadcasted to the other
   connections, and the choose function is run to determine whether this
   elevator should take the order */
void Logic::notify(const ExternalButtonEvent& event)
{
  network.event_queue.push(NetworkMessageEvent<ExternalButtonEvent>("all", event));
  choose_elevator(event.floor, event.type);
}

/* When a state update event occurs, our own state in elevator_infos should be
   updated, and it should be broadcasted to the other elevators. */
void Logic::notify(const StateUpdateEvent& event)
{
  elevator_infos["me"] = { true, event.state };
  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>("all", event));
  backup_orders(event.state.orders);
}

/* When a state update event is received from the network, it should be stored
   in elevator_infos, creating a new index if needed. */
void Logic::notify(const NetworkMessageEvent<StateUpdateEvent>& event)
{
  LOG_DEBUG("Received " << event);
  if (elevator_infos.find(event.ip) != elevator_infos.end())
    elevator_infos[event.ip] = { true, event.data.state };
}

/* When an external button event is received from the network, the choose
   function is run to determine whether this elevator should take the order. */
void Logic::notify(const NetworkMessageEvent<ExternalButtonEvent>& event)
{
  LOG_DEBUG("Received " << event);
  choose_elevator(event.data.floor, event.data.type);
}

/* When a connection is lost, that elevator's state should be removed from
   elevator_infos, since it should no longer be part of the decision making */
void Logic::notify(const LostConnectionEvent& event)
{
  elevator_infos[event.ip].active = false;
}

/* When a new elevator appears we must send our latest state to it. 

   If the elevator is known from before, it means that it has died and
   come back. In this case, we send it the most recent order list this
   elevator gave us before it died.
*/
void Logic::notify(const NewConnectionEvent& event)
{
  LOG_DEBUG("Sending state update request");
  network.event_queue.push(NetworkMessageEvent<StateUpdateReqEvent>(event.ip, {}));
  elevator_infos[event.ip] = { false, {} };
}

/* When our own network connection is lost, remove all elevator states
   except our own, as the information about the rest of the elevators
   will be outdated. */
void Logic::notify(const LostNetworkEvent&)
{
  for (auto& pair : elevator_infos) {
    if (pair.first != "me")
      pair.second.active = false;
  }
}

void Logic::notify(const NetworkMessageEvent<StateUpdateReqEvent>& event)
{
  LOG_DEBUG("Got state update request, sending state.");
  StateUpdateEvent state(elevator_infos["me"].state);
  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>(event.ip, state));
}

void Logic::backup_orders(const std::vector<std::vector<bool>>& orders)
{
  assert(orders.size() == FLOORS && orders[0].size() == 3); 
  std::ofstream of(backup_filename);
  for (int floor = 0; floor < FLOORS; floor++) {
    for (int type = 0; type <= 2; type++) {
      of << orders[floor][type] << " ";
    }
    of << std::endl;
  }
}

bool Logic::restore_orders(std::vector<std::vector<bool>>& orders)
{
  std::ifstream ifs(backup_filename);
  if (ifs.fail()) {
    return false;
  }
  for (int floor = 0; floor < FLOORS; floor++) {
    for (int type = 0; type <= 2; type++) {
      bool val;
      ifs >> val;
      orders[floor][type] = val;
    }
  }
  return true;
}

void Logic::run()
{
  while (true) {
    event_queue.handle_events(event_queue.wait());
  };
}
