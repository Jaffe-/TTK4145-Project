
#include <stdio.h>

#include "../hw_interface/elev.h"

int main() {
    elev_init();

    printf("Press STOP button to stop elevator and exit program.\n");

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

	while(1) {
		
	}
}
