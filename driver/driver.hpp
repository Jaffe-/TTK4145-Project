#include <vector>
#include <chrono>

#define FLOORS 4

enum class Button {
  INTERNAL_1, INTERNAL_2, INTERNAL_3, INTERNAL_4,
  EXTERNAL_1U,
  EXTERNAL_2D, EXTERNAL_2U,
  EXTERNAL_3D, EXTERNAL_3U,
  EXTERNAL_4D
};

bool is_internal(Button button);
unsigned int internal_button_floor(Button button);

class DriverEvent {
public:
  enum Type {
    BUTTON_PRESS, FLOOR_SIGNAL
  };

  Type type;
  Button button;
  unsigned int floor;
};

class FSM {
public:
  void notify(DriverEvent event);
  void run();

private:
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

  enum State {
    MOVING, STOPPED
  };
  enum Direction {
    UP, DOWN, STOP
  };

  bool should_stop(int floor);
  void change_state(State new_state);
  bool floors_below();
  bool floors_above();
  void update_lights();
  
  State state;
  int current_floor;
  Direction direction;
  bool orders[FLOORS][3];
  bool door_open;
  const std::chrono::duration<double> door_time = std::chrono::seconds(3);
  TimePoint door_opened_time;
};

class Driver {
public:
  Driver(bool use_simulator);
  
  void update_floors(std::vector<unsigned int> new_floors);
  void run();

  
private:
  void notify_fsm(DriverEvent event);
  void insert_order(unsigned int floor);

  FSM fsm;
};
