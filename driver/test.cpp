#include "hw_interface/elev.h"
#include "test.hpp"
#include "../logger/logger.hpp"

Driver::Driver(bool use_simulator)
{
  std::string driver_string = use_simulator ? "simulated" : "hardware";
  if (elev_init(use_simulator ? ET_simulation : ET_comedi)) {
    LOG_DEBUG("Started driver (" << driver_string << ")");
  }
  else {
    LOG_DEBUG("Failed to start driver (" << driver_string << ")");
    // Exception?
  }
}

void Driver::run()
{

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
