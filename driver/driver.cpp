#include "hw_interface/elev.h"
#include "driver.hpp"
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

void Driver::insert_order(unsigned int floor)
{
  
}

void Driver::notify_fsm(DriverEvent event)
{
  if (event.type == BUTTON_PRESS) {
    
  }
  else if (event.type == FLOOR_SIGNAL) {

  }
}

void Driver::run()
{
  /*
  switch (state) {
  case FSMState::MOVING:
    if (elev_get_floor_sensor_signal() == floors[0]) {
      elev_set_motor_direction(DIRN_STOP);
      state = FSMState::STOPPED;
    }
  case FSMState::STOPPED:
    
  }
  */
}
