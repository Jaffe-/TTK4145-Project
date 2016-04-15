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

  if (restore_orders())
    LOG_DEBUG("Restored internal orders");
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
      LOG(4, "Calculated steps " << steps << " for id " << ip);
      if (steps < min) {
	min = steps;
	min_ip = ip;
      }
    }
  }

  assert(min != INT_MAX);

  LOG_DEBUG("Order " << order_id << ": " << min_ip << " is chosen (" << min << " steps)");
  if (min_ip == network.own_ip()) {
    driver.event_queue.push(OrderUpdateEvent(floor, static_cast<int>(type)));
  }

  assert(orders.find(order_id) != orders.end() && "Computed best ip for a order that doesn't exist");
  orders[order_id].owner = min_ip;

  LOG(4, "Order map now contains: ");
  for (auto& pair : orders) {
    LOG(4, pair.first << ": floor=" << pair.second.floor << " type=" << pair.second.type << " owner=" << pair.second.owner);
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
  backup_orders();
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
  for (auto& order_pair : orders) {
    if (order_pair.second.owner == event.ip) {
      choose_elevator(order_pair.first, order_pair.second.floor, static_cast<ButtonType>(order_pair.second.type));
    }
  }
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
      LOG_DEBUG("Order " << it->first << ": completed by this elevator");
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
    LOG_DEBUG("Order " << event.data.id << ": " << event.ip << " reports that order is completed");
    orders.erase(it);
  }
}

/* Writes internal orders to file */
void Logic::backup_orders()
{
  const auto& orders = elevator_infos[network.own_ip()].state.orders;
  assert(orders.size() == FLOORS && orders[0].size() == 3);
  std::ofstream of(backup_filename);
  for (int floor = 0; floor < FLOORS; floor++) {
    of << orders[floor][2] << " ";
  }
}

/* Restores internal orders from file */
bool Logic::restore_orders()
{
  std::ifstream ifs(backup_filename);
  if (ifs.fail()) {
    return false;
  }

  std::vector<bool> orders = {0,0,0,0};
  for (int floor = 0; floor < FLOORS; floor++) {
    bool val;
    ifs >> val;
    orders[floor] = val;
  }

  bool restored;
  for (int floor = 0; floor < FLOORS; floor++) {
    if (orders[floor]) {
      restored = true;
      driver.event_queue.push(OrderUpdateEvent(floor, 2));
    }
  }

  return restored;
}

void Logic::run()
{
  while (true) {
    event_queue.handle_events(event_queue.wait(), this, events);
  };
}
