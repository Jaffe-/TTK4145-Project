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
		     NetworkMessageEvent<StateUpdateReqEvent>,
		     FSMOrderCompleteEvent,
		     NetworkMessageEvent<OrderCompleteEvent>>());

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
void Logic::choose_elevator(const std::string& order_id, int floor, ButtonType type)
{
  int min = INT_MAX;
  std::string min_ip;

  std::vector<std::string> sorted_ips;
  for (auto pair : elevator_infos) {
    sorted_ips.push_back(pair.first);
  }
  std::sort(sorted_ips.begin(), sorted_ips.end());

  for (auto& ip : sorted_ips) {
    const ElevatorInfo& elevator_info = elevator_infos[ip];
    if (elevator_info.active) {
      int steps = SimulatedFSM(elevator_info.state).calculate(floor, static_cast<int>(type));
      LOG_DEBUG("Calculated steps " << steps << " for id " << ip);
      if (steps < min) {
	min = steps;
	min_ip = ip;
      }
    }
  }

  assert(min != INT_MAX);

  if (min_ip == network.own_ip()) {
    driver.event_queue.push(OrderUpdateEvent(floor, static_cast<int>(type)));
  }

  assert(orders.find(order_id) != orders.end() && "Computed best ip for a order that doesn't exist");
  orders[order_id].owner = min_ip;

  LOG_DEBUG("Order map now contains: ");
  for (auto& pair : orders) {
    LOG_DEBUG(pair.first << ": floor=" << pair.second.floor << " type=" << pair.second.type << " owner=" << pair.second.owner);
  }
}

/* When an external button event occurs, it is broadcasted to the other
   connections, and the choose function is run to determine whether this
   elevator should take the order */
void Logic::notify(const ExternalButtonEvent& event)
{
  std::string order_id = network.own_ip() + ":" + std::to_string(current_id++);
  orders[order_id] = { event.floor, static_cast<int>(event.type), "" };
  choose_elevator(order_id, event.floor, event.type);
  network.event_queue.push(NetworkMessageEvent<ExternalButtonEvent>("all", { event.floor, event.type, order_id }));
}

/* When a state update event occurs, our own state in elevator_infos should be
   updated, and it should be broadcasted to the other elevators. */
void Logic::notify(const StateUpdateEvent& event)
{
  elevator_infos[network.own_ip()] = { true, event.state };
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
  orders[event.data.id] = { event.data.floor, static_cast<int>(event.data.type), ""};
  choose_elevator(event.data.id, event.data.floor, event.data.type);
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
    if (pair.first != network.own_ip())
      pair.second.active = false;
  }
}

void Logic::notify(const NetworkMessageEvent<StateUpdateReqEvent>& event)
{
  LOG_DEBUG("Got state update request, sending state.");
  StateUpdateEvent state(elevator_infos[network.own_ip()].state);
  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>(event.ip, state));
}

void Logic::notify(const FSMOrderCompleteEvent& event)
{
  LOG_DEBUG("Received " << event);
  for (auto it = orders.begin(); it != orders.end(); ) {
    if (it->second.floor == event.floor && it->second.type == event.type) {
      LOG_DEBUG("Found matching order id " << it->first);
      network.event_queue.push(NetworkMessageEvent<OrderCompleteEvent>("all", it->first));
      it = orders.erase(it);
    }
    else {
      ++it;
    }
  }
}

void Logic::notify(const NetworkMessageEvent<OrderCompleteEvent>& event)
{
  auto it = orders.find(event.data.id);
  if (it != orders.end()) {
    LOG_DEBUG("Order " << event.data.id << " is complete and was ERASED");
    orders.erase(it);
  }
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
