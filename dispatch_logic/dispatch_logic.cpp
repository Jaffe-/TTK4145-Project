#include "dispatch_logic.hpp"
#include "events.hpp"
#include "../util/logger.hpp"
#include "../driver/events.hpp"
#include "../network/events.hpp"
#include "simulatedfsm.hpp"
#include <climits>
#include <string>
#include <fstream>

DispatchLogic::DispatchLogic(bool use_simulator, const std::string& port)
  : driver(event_queue, use_simulator),
    network(event_queue, port),
    driver_thread([&] { driver.run(); }),
    network_thread([&] { network.run(); })
{
  LOG_INFO("DispatchLogic module started");

  if (restore_orders())
    LOG_DEBUG("Restored internal orders");
}



/* --- Events involved in adding and completing orders --- */

/* When an external button event occurs, a new order is created (if it doesnt already exist)
   and choose_elevator is called to determine which elevator is going to take
   the order. */
void DispatchLogic::notify(const ExternalButtonEvent& event)
{
  if (!order_exists(event.floor, static_cast<int>(event.type))) {
    std::string order_id = network.own_ip() + ":" + std::to_string(current_id++);
    choose_elevator(order_id, event.floor, event.type);
  }
  else {
    LOG(4, "Order for " << event << " already exists.");
  }
}

/* When an order taken event is received, we check if it is assigned to us, and
   send an order update event to the driver if it is. The order is added to the
   order map. */
void DispatchLogic::notify(const NetworkMessageEvent<NewOrderEvent>& event)
{
  add_order(event.data.id, event.data.info);
  LOG_DEBUG("New order " << event.data.id << ": taken by " << event.data.info.owner);
}

/* When the FSM tells us that it has completed an order, we remove it from the
   system and send an OrderCompleteEvent to the other elevators. */
void DispatchLogic::notify(const FSMOrderCompleteEvent& event)
{
  LOG_DEBUG("Received " << event);
  for (auto it = orders.begin(); it != orders.end(); ) {
    if (it->second.floor == event.floor && it->second.type == event.type) {
      LOG_DEBUG("Order " << it->first << ": completed by this elevator");
      driver.event_queue.push(ExternalLightOffEvent
			      (event.floor, static_cast<ButtonType>(event.type)));
      network.event_queue.push(NetworkMessageEvent<OrderCompleteEvent>
			       ("all", it->first));
      it = orders.erase(it);
    }
    else {
      ++it;
    }
  }
}

/* When we receive an OrderCompleteEvent, the order is removed from the order
   map */
void DispatchLogic::notify(const NetworkMessageEvent<OrderCompleteEvent>& event)
{
  auto it = orders.find(event.data.id);
  if (it != orders.end()) {
    driver.event_queue.push(ExternalLightOffEvent
			    (it->second.floor, static_cast<ButtonType>(it->second.type)));
    LOG_DEBUG("Order " << event.data.id << ": " << event.ip << " reports that order is completed");
    orders.erase(it);
  }
}



/* --- Events involved in communicating the 'state of the world' --- */

/* When we receive an update request, we get our own most recent state
   from elevators, and our current order map and send them to the
   elevator which requested it. */
void DispatchLogic::notify(const NetworkMessageEvent<UpdateRequestEvent>& event)
{
  LOG_DEBUG("Received update request, sending state and order map.");
  StateUpdateEvent state(elevators[network.own_ip()].state);

  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>
			   (event.ip, state));

  network.event_queue.push(NetworkMessageEvent<OrderMapUpdateEvent>
			   (event.ip, OrderMapUpdateEvent(orders)));
}

/* When a state update event occurs, our own state in the elevators map should be
   updated, and it should be sent to the other elevators. If the error
   flag is set, there is some problem with the driver/fsm. We then remove
   ourselves (mark ourselves as inactive) so we don't participate in order
   dispatching. */
void DispatchLogic::notify(const StateUpdateEvent& event)
{
  if (!event.state.error)
    elevators[network.own_ip()] = { true, event.state };
  else {
    remove_elevator(network.own_ip());
    LOG_ERROR("Driver error!");
  }
  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>("all", event));
  backup_orders();
}

/* When a state update is received from the network, we only add it if the
   elevator exists in elevators map (it is known to us). If the new state has
   the error flag set, the elevator is removed (marked as inactive). Otherwise
   the previous state is overwritten with the new one. */
void DispatchLogic::notify(const NetworkMessageEvent<StateUpdateEvent>& event)
{
  LOG_DEBUG("Received " << event);
  if (elevators.find(event.ip) != elevators.end()) {
    if (!event.data.state.error)
      elevators[event.ip] = { true, event.data.state };
    else {
      remove_elevator(event.ip);
      LOG_WARNING("Elevator " << event.ip << " has driver problems.");
    }
  }
}

/* When an order map update is received (which happens when a new connection is
   discovered) we merge it with our own order map */
void DispatchLogic::notify(const NetworkMessageEvent<OrderMapUpdateEvent>& event)
{
  LOG_DEBUG("Received order map update from " << event.ip);
  for (const auto& pair : event.data.orders) {
    if (!order_exists(pair.second.floor, pair.second.type)) {
      LOG(4, "Adding order " << pair.first);
      orders[pair.first] = pair.second;
    }
  }
}



/* --- Events involved in adding and removing connections --- */

/* When a connection is lost, that elevator is removed (marked as inactive) from
   the decision process. */
void DispatchLogic::notify(const LostConnectionEvent& event)
{
  remove_elevator(event.ip);
}

/* When a new connection appears, we ask it for an update by sending a
   UpdateRequestEvent. The elevator is marked as inactive until those updates
   arrive. */
void DispatchLogic::notify(const NewConnectionEvent& event)
{
  LOG_DEBUG("Sending state update and order map update requests");
  network.event_queue.push(NetworkMessageEvent<UpdateRequestEvent>(event.ip, {}));
  elevators[event.ip] = { false, {} };
}



/* Calculate the cost function (using the simulated FSM) for each elevator and
   determine which elevator should take it. */
void DispatchLogic::choose_elevator(const std::string& order_id, int floor, ButtonType type)
{
  int min = INT_MAX;
  std::string min_ip;

  for (const auto& pair : elevators) {
    const ElevatorInfo& elevator_info = pair.second;
    if (elevator_info.active) {
      int steps = SimulatedFSM(elevator_info.state).calculate(floor, static_cast<int>(type));
      LOG(4, "Calculated steps " << steps << " for id " << pair.first);
      if (steps < min) {
        min = steps;
        min_ip = pair.first;
      }
    }
  }

  // We found no elevator to take the order
  if (min == INT_MAX) {
    return;
  }

  LOG_DEBUG("Order " << order_id << ": " << min_ip << " is chosen (" << min << " steps)");

  auto order = OrderInfo { floor, static_cast<int>(type), min_ip };
  add_order(order_id, order);
  network.event_queue.push(NetworkMessageEvent<NewOrderEvent>
			   ("all", NewOrderEvent { order_id, order }));

}

void DispatchLogic::add_order(const std::string& id, const OrderInfo& info)
{
  orders[id] = info;

  driver.event_queue.push(ExternalLightOnEvent(info.floor, static_cast<ButtonType>(info.type)));
  if (info.owner == network.own_ip()) {
    LOG_DEBUG("Order " << id << ": this elevator takes the order");
    driver.event_queue.push(OrderUpdateEvent(info.floor, info.type));
  }
}

/* Check whether an order with the given floor and type (direction) exists
   already. This is necessary to avoid duplicate orders in the system. */
bool DispatchLogic::order_exists(int floor, int type)
{
  return std::find_if(orders.begin(), orders.end(),
		      [floor, type] (const std::pair<std::string, OrderInfo> &o) {
			return o.second.floor == floor && o.second.type == type;
		      }) != orders.end();
}

/* Mark elevator as inactive in the elevators map and determine whether this
   elevator should take on the job of reassigning the orders that the dead
   elevator owned. */
void DispatchLogic::remove_elevator(const std::string& ip)
{
  LOG_DEBUG("Distributing orders for dead elevator " << ip);

  elevators[ip].active = false;

  std::vector<std::string> ips;
  for (auto pair : elevators) {
    if (pair.second.active)
      ips.push_back(pair.first);
  }

  auto min_ip_it = std::min_element(ips.begin(), ips.end());

  // The remaining elevator with the lowest IP address should be responsible for
  // assigning the orders of the dead elevator to new owners.
  if (min_ip_it != ips.end() && *min_ip_it == network.own_ip()) {
    LOG_DEBUG("This elevator should do the order reassigning");
    for (auto& order_pair : orders) {
      if (order_pair.second.owner == ip) {
	LOG_DEBUG("Order " << order_pair.first << " belonged to disconnected elevator and needs to be reassigned");
	choose_elevator(order_pair.first, order_pair.second.floor, static_cast<ButtonType>(order_pair.second.type));
      }
    }
  }
}

/* Writes internal orders to file */
void DispatchLogic::backup_orders()
{
  const auto& orders = elevators[network.own_ip()].state.orders;
  assert(orders.size() == FLOORS && orders[0].size() == 3);
  std::ofstream of(backup_filename);
  for (int floor = 0; floor < FLOORS; floor++) {
    of << orders[floor][2] << " ";
  }
}

/* Restores internal orders from file */
bool DispatchLogic::restore_orders()
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

void DispatchLogic::run()
{
  while (true) {
    event_queue.handle_events(event_queue.wait(), this, events);
  };
}
