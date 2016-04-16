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
  if (!order_exists(event.floor, static_cast<int>(event.type))) {
    std::string order_id = network.own_ip() + ":" + std::to_string(current_id++);
    network.event_queue.push(NetworkMessageEvent<ExternalButtonEvent>("all", { event.floor, event.type, order_id }));

    add_order(order_id, event.floor, event.type);
  }
  else {
    LOG(4, "Order for " << event << " already exists.");
  }
}

/* When an external button press event is received from the network, the
   corresponding order id is added to our order map. The order id has been
   generated on the sender side. */
void Logic::notify(const NetworkMessageEvent<ExternalButtonEvent>& event)
{
  LOG_DEBUG("Received " << event);
  driver.event_queue.push(event.data);
   add_order(event.data.id, event.data.floor, event.data.type);
}


/* When we receive a state update request, we get our own most recent state
   from elevator_infos and send it to the elevator which requested it. */
void Logic::notify(const NetworkMessageEvent<StateUpdateReqEvent>& event)
{
  LOG_DEBUG("Got state update request, sending state.");
  StateUpdateEvent state(elevator_infos[network.own_ip()].state);
  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>(event.ip, state));
}

/* When a state update event occurs, our own state in elevator_infos should be
   updated, and it should be broadcasted to the other elevators. If the error
   flag is set, there is some problem with the driver/fsm. We then remove
   ourselves (mark ourselves as inactive) so we don't participate in order
   dispatching. */
void Logic::notify(const StateUpdateEvent& event)
{
  if (!event.state.error)
    elevator_infos[network.own_ip()] = { true, event.state };
  else {
    remove_elevator(network.own_ip());
    LOG_ERROR("Driver error!");
  }
  network.event_queue.push(NetworkMessageEvent<StateUpdateEvent>("all", event));
  backup_orders();
}

/* When a state update is received from the network, we only add it if the
   elevator exists in elevator_infos (it is known to us). If the new state has
   the error flag set, the elevator is removed (marked as inactive). Otherwise
   the previous state is overwritten with the new one. */
void Logic::notify(const NetworkMessageEvent<StateUpdateEvent>& event)
{
  LOG_DEBUG("Received " << event);
  if (elevator_infos.find(event.ip) != elevator_infos.end()) {
    if (!event.data.state.error)
      elevator_infos[event.ip] = { true, event.data.state };
    else {
      remove_elevator(event.ip);
      LOG_INFO("Elevator " << event.ip << " has driver problems.");
    }
  }
}


/* When a connection is lost, that elevator is removed (marked as inactive) from
   the decision process. */
void Logic::notify(const LostConnectionEvent& event)
{
  remove_elevator(event.ip);
}

/* When a new connection appears, we ask it for a state update by sending a
   StateUpdateReqEvent. The elevator is marked as inactive until that state
   update comes. */
void Logic::notify(const NewConnectionEvent& event)
{
  LOG_DEBUG("Sending state update and order map update requests");
  network.event_queue.push(NetworkMessageEvent<StateUpdateReqEvent>(event.ip, {}));
  network.event_queue.push(NetworkMessageEvent<OrderMapReqEvent>(event.ip, {}));
  elevator_infos[event.ip] = { false, {} };
}

/* When our own network connection is lost, we mark all other elevators as
   inactive so that their outdated state information won't affect the operation
   of this elevator */
void Logic::notify(const LostNetworkEvent&)
{
  for (auto& pair : elevator_infos) {
    if (pair.first != network.own_ip())
      pair.second.active = false;
  }
}


/* When the FSM tells us that it has completed an order, we remove it from the
   system and send an OrderCompleteEvent to the other elevators. */
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

/* When we receive an OrderCompleteEvent, the order is removed from the order
   map */
void Logic::notify(const NetworkMessageEvent<OrderCompleteEvent>& event)
{
  auto it = orders.find(event.data.id);
  if (it != orders.end()) {
    driver.event_queue.push(FSMOrderCompleteEvent(it->second.floor, it->second.type));
    LOG_DEBUG("Order " << event.data.id << ": " << event.ip << " reports that order is completed");
    orders.erase(it);
  }
}


void Logic::notify(const NetworkMessageEvent<OrderMapReqEvent>& event)
{
  LOG_DEBUG("Received order map update request");
  network.event_queue.push(NetworkMessageEvent<OrderMapUpdateEvent>
			   (event.ip, OrderMapUpdateEvent(orders)));
}

void Logic::notify(const NetworkMessageEvent<OrderMapUpdateEvent>& event)
{
  LOG_DEBUG("Received order map update from " << event.ip);
  for (const auto& pair : event.data.orders) {
    if (!order_exists(pair.second.floor, pair.second.type)) {
      LOG(4, "Adding order " << pair.first);
      orders[pair.first] = pair.second;
    }
  }
}


/* Check whether an order with the given floor and type (direction) exists
   already. This is necessary to avoid duplicate orders in the system. */
bool Logic::order_exists(int floor, int type)
{
  return std::find_if(orders.begin(), orders.end(),
		      [floor, type] (const std::pair<std::string, OrderInfo> &o) {
			return o.second.floor == floor && o.second.type == type;
		      }) != orders.end();
}

/* Add the order with the given id */
void Logic::add_order(const std::string& id, int floor, ButtonType btype)
{
  int type = static_cast<int>(btype);
  orders[id] = { floor, type, ""};
  choose_elevator(id, floor, btype);
}

/* Remove elevator for elevator_infos and go through each order and choose a new
   elevator to take it */
void Logic::remove_elevator(const std::string& ip)
{
  elevator_infos[ip].active = false;
  LOG_DEBUG("Remove ip " << ip);
  for (auto& order_pair : orders) {
    if (order_pair.second.owner == ip) {
      choose_elevator(order_pair.first, order_pair.second.floor, static_cast<ButtonType>(order_pair.second.type));
    }
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
