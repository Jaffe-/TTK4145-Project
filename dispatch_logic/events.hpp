#pragma once

#include "../util/event.hpp"
#include <vector>

struct OrderInfo {
  int floor;
  int type;
  std::string owner;
};

struct UpdateRequestEvent : public SerializableEvent {
  UpdateRequestEvent() {};
  UpdateRequestEvent(const json_t&) {};
};

struct NewOrderEvent : public SerializableEvent {
  NewOrderEvent(const std::string& id, OrderInfo info) : id(id), info(info) {}
  NewOrderEvent(const json_t& json) {
    id = json["id"];
    info = { json["floor"], json["type"], json["owner"] };
  }

  virtual json_t get_json() const override {
    return {{"id", id},
    	    {"floor", info.floor},
	    {"type", info.type},
	    {"owner", info.owner}};
  }

  std::string id;
  OrderInfo info;
};

struct OrderCompleteEvent : public SerializableEvent {
  OrderCompleteEvent(const std::string& id) : id(id) {}
  OrderCompleteEvent(const json_t& json) {
    id = json["id"];
  }

  virtual json_t get_json() const override {
    return {{"id", id}};
  }

  std::string id;
};

struct OrderMapUpdateEvent : public SerializableEvent {
  OrderMapUpdateEvent(const std::map<std::string, OrderInfo>& orders) : orders(orders) {};
  OrderMapUpdateEvent(const json_t& js) {
    json_t json = js;
    for (json_t::iterator it = json.begin(); it != json.end(); ++it) {
      orders[it.key()] = OrderInfo { it.value()["floor"],
				     it.value()["type"],
				     it.value()["owner"] };
    }
  }

  virtual json_t get_json() const override {
    json_t json;
    for (const auto& pair : orders) {
      json_t entry_json = {
	{"floor", pair.second.floor},
	{"type", pair.second.type},
	{"owner", pair.second.owner}
      };
      json[pair.first] = entry_json;
    }
    return json;
  }

  std::map<std::string, OrderInfo> orders;
};
