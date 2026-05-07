/* servo_pwm.c — TIM1 PWM 50 Hz × 12 channel servo driver.
 *
 * Source: dog_robot/dog-robot-test.md servo subsystem + .roadmap.hexa_bot
 * §A.1 σ(6)=12 (12 servo channels = 4 legs × 3 joints).
 *
 * The "apply" entry copies dog_state.servo_cmd[] (computed by gait_sm.c)
 * into TIM1 CCRn registers in the order:
 *
 *   CCRn = TIM_neutral_count + servo_cmd / 2   (for n in 1..4 + extension)
 *
 * mk1 STM32F4 has 4 TIM1 + 4 TIM8 + 4 TIM3 channels — 12 total. Production
 * firmware spreads servo_cmd across all three timers; the scaffold only
 * shows the TIM1 path.
 */
#include "stm32f4xx_min.h"
#include "dog_robot.h"

#define SERVO_NEUTRAL_COUNT  18000u   /* 1.5 ms @ 60 MHz tick base       */
#define SERVO_RANGE_HALF     6000u    /* ±0.5 ms swing → ±90° servo arc  */

#if !defined(SIM_BUILD)
static void timN_write(volatile uint32_t *ccr, int16_t cmd) {
    /* Map ±SERVO_PWM_MAX (±18000) onto ±SERVO_RANGE_HALF (±6000) — 1/3
     * scale to keep micro-servo arc within mechanical limits. */
    int32_t scaled = (int32_t)cmd / 3;
    int32_t count  = (int32_t)SERVO_NEUTRAL_COUNT + scaled;
    if (count < 0) count = 0;
    *ccr = (uint32_t)count;
}
#endif

void dog_servo_apply(void) {
#if defined(SIM_BUILD)
    /* Sim: no-op — sscb_sim.c reads dog_state.servo_cmd[] directly. */
    (void)dog_state;
#else
    /* TIM1 ch1..ch4 → legs FL/FR (hip_yaw + hip_pitch).
     * Production firmware also drives TIM8 + TIM3 for the remaining 8
     * channels; the scaffold only wires TIM1 to keep the example readable. */
    timN_write(&TIM1->CCR1, dog_state.servo_cmd[dog_joint_index(LEG_FL, JOINT_HIP_YAW)]);
    timN_write(&TIM1->CCR2, dog_state.servo_cmd[dog_joint_index(LEG_FL, JOINT_HIP_PITCH)]);
    timN_write(&TIM1->CCR3, dog_state.servo_cmd[dog_joint_index(LEG_FR, JOINT_HIP_YAW)]);
    timN_write(&TIM1->CCR4, dog_state.servo_cmd[dog_joint_index(LEG_FR, JOINT_HIP_PITCH)]);
    /* TODO Mk.II: extend with TIM8 + TIM3 channels (knee + rear legs).   */
#endif
}
