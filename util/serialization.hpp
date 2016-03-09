#include "json.hpp"

using json_t = nlohmann::json;

class Serializable {
public:
  virtual json_t get_json() const = 0;
  //  virtual void deserialize(const std::string& s) = 0;
  std::string serialize() const {
    return get_json().dump();
  }
};
