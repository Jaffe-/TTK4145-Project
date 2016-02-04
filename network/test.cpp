#include "receiver.hpp"

int main()
{
  Network::start("30021");
  Network::receiver.run();
}
