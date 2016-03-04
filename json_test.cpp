#include <iostream>
#include "util/json.hpp"

using json = nlohmann::json;

class Serializable {
public:
  virtual std::string serialize() = 0;
  virtual void deserialize(std::string s) = 0;
};

class A : public Serializable {
  //private:
public:
  int a;
  std::string b;
  double c;
  A(std::string s) {
    deserialize(s);
  }
  std::string serialize() {
    json js = {{"a", a},
	       {"b", b},
	       {"c", c}};
    return js.dump();
  }

  void deserialize(std::string s) {
    json js = json::parse(s);
    a = js["a"];
    b = js["b"];
    c = js["c"];
  }
};

int main()
{
  json test = {{"a", 2},
	       {"b", "hei"},
	       {"c", 5.4}};
  A tst(test.dump());
  //  std::string raw = test.dump();
  std::cout << tst.a << std::endl;
  std::cout << tst.b << std::endl;
  std::cout << tst.c << std::endl;

  Serializable* p = &tst;
  
  std::cout << p->serialize() << std::endl;
  
}
