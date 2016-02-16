#include "hw_interface/elev.h"

class Driver {
public:
  void run();
private:
  
};

void Driver::run()
{
#ifdef USE_SIMULATOR
  elev_init(ET_simulation);
#else
  elev_init(ET_comedi);
#endif

  return;
}
