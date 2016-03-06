#include <iostream>
#include "util/json.hpp"
#include "util/serialization.hpp"
#include "util/message_queue.hpp"

using json = nlohmann::json;

class A : public Serializable {
  //private:
public:
  int a;
  std::string b;
  double c;
  explicit A(const json& js) {
    a = js["a"];
    b = js["b"];
    c = js["c"];
  };

  //A(const std::string& s) : A(json::parse(s)) {};

  json get_json() const {
    return {{"a", a},
	    {"b", b},
	    {"c", c}};
  }
};

class B {
  int a;
public:
};

void send(const Serializable& bm)
{
  std::cout << bm.serialize() << std::endl;
}

int main()
{
  json test = {{"a", 2},
	       {"b", "hei"},
	       {"c", 5.4}};
  A tst(test);

  SerializableMessage<A> tst_msg(tst);
  std::cout << tst_msg.get_type().name() << std::endl;

  const Message<A>& b = tst_msg;
  
  //  std::string raw = test.dump();
  std::cout << tst.a << std::endl;
  std::cout << tst.b << std::endl;
  std::cout << tst.c << std::endl;

  const BaseMessage& base_msg = tst_msg;
  send(base_msg);

  SerializableMessage<A> recv(tst_msg.serialize());
  std::cout << recv.data.a << std::endl;
  std::cout << recv.data.b << std::endl;
  std::cout << recv.data.c << std::endl;

  Message<B> btst = B();
  const BaseMessage& bbase = btst;
}
