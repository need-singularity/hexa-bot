/* main.c — dog_robot mk1 firmware entry.
 *
 * Source: dog_robot/dog-robot-test.md (spec) + .roadmap.hexa_bot §A.1
 * (n=6 lattice). Pattern source: hexa-sscb/module/firmware/src/main.c.
 */
#include "stm32f4xx_min.h"
#include "dog_robot.h"

struct dog_state dog_state;

#if !defined(SIM_BUILD)
int main(void) {
    dog_system_init();
    dwt_cycle_counter_enable();

    dog_state.state_id = DOG_STATE_BOOT;
    dog_state.pattern  = GAIT_PATTERN_STAND;
    dog_main_loop();

    /* not reached */
    return 0;
}

void dog_main_loop(void) {
    for (;;) {
        /* mk1: foreground super-loop. Cycle is ~5 ms (200 Hz outer loop).
         * Servo PWM update is timer-driven (TIM1) at 50 Hz so apply step
         * runs every 20 ms — 1-in-4 main_loop iterations. */
        dog_gait_step(5);
        dog_servo_apply();
        dog_fault_check();
        dog_state.tick_ms += 5;
    }
}
#endif /* !SIM_BUILD */

void dog_fault_check(void) {
    if (dog_state.flags) {
        dog_state.state_id = DOG_STATE_FAULT;
    }
}
