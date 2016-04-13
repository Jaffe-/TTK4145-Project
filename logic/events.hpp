#pragma once

#include "../util/event.hpp"
#include <vector>

struct OrderBackupEvent : public SerializableEvent {
  std::vector<std::vector<bool>> orders;

  OrderBackupEvent(const std::vector<std::vector<bool>>& orders) : orders(orders)
  {
  }

  OrderBackupEvent(const json_t json)
    : orders(json["orders"].get<std::vector<std::vector<bool>>>())
  {
  }

  virtual json_t get_json() const override {
    return {{"orders", orders}};
  }
};
