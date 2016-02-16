#include "hw_interface/elev.h"
#include "test.hpp"

void Driver::run()
{
#ifdef USE_SIMULATOR
  elev_init(ET_simulation);
#else
  elev_init(ET_comedi);
#endif

  elev_set_motor_direction(DIRN_UP);
  while (1) {
    if (elev_get_floor_sensor_signal() == N_FLOORS - 1) {
      elev_set_motor_direction(DIRN_STOP);
      break;
    }
    /*
    // Change direction when we reach top/bottom floor
    if (elev_get_floor_sensor_signal() == N_FLOORS - 1) {
    elev_set_motor_direction(DIRN_DOWN);
    } else if (elev_get_floor_sensor_signal() == 0) {
    elev_set_motor_direction(DIRN_UP);
    }

    // Stop elevator and exit program if the stop button is pressed
    if (elev_get_stop_signal()) {
    elev_set_motor_direction(DIRN_STOP);
    return 0;
    } */
  }

  return;
}
