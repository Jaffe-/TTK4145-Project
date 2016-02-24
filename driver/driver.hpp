#include <vector>

enum class Button {
  INTERNAL_1, INTERNAL_2, INTERNAL_3, INTERNAL_4,
  EXTERNAL_1U,
  EXTERNAL_2D, EXTERNAL_2U,
  EXTERNAL_3D, EXTERNAL_3U,
  EXTERNAL_4D
}

class DriverEvent {
public:
  enum Type {
    BUTTON_PRESS, FLOOR_SIGNAL
  };

  Button button;
  unsigned int floor;
};

class Driver {
public:
  Driver(bool use_simulator);
  
  void update_floors(std::vector<unsigned int> new_floors);
  void run();

  enum class FSMState {
    MOVING, STOPPED
  };
  
private:
  void notify_fsm(DriverEvent event);
  void insert_order(unsigned int floor);

  std::vector<unsigned int> floors;
  unsigned int current_floor;
  FSMState state;
  elev_motor_direction_t direction;
};
