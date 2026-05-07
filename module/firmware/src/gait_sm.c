/* gait_sm.c — quadruped gait state machine (τ(6)=4 phase cycle).
 *
 * Implements the four-phase gait pattern table for STAND / TROT, plus the
 * Mk.II patterns BOUND / GALLOP (compile-time guarded by GAIT_PATTERN_*
 * enum values; mk1 only invokes STAND + TROT). Source: dog_robot/dog-
 * robot-test.md §τ(6)=4 gait cycle + .roadmap.hexa_bot §A.1.
 *
 * The gait phase wheel for a leg is parameterised by per-leg phase
 * offsets (in 1/256-cycle units) + the global gait wheel position. The
 * table below encodes which phase each leg should be in for each pattern
 * — the n=6 lattice projection (4 phases × 4 legs = σ(6)+τ(6) bookkeeping).
 */
#include "dog_robot.h"

/* Per-pattern leg phase offsets, in 1/256-cycle units (Q256).
 *
 *   STAND  — all legs grounded; gait wheel does not advance.
 *   TROT   — diagonal pairs (FL↔RR, FR↔RL) move together (180° offset).
 *   BOUND  — front pair vs rear pair (180° offset).
 *   GALLOP — sequential FL → FR → RL → RR (90° spacing).
 *
 * Indexed [pattern][leg].
 */
static const uint8_t LEG_PHASE_OFFSET_Q256[4][DOG_LEG_COUNT] = {
    /* STAND  */ {   0,   0,   0,   0   },
    /* TROT   */ {   0, 128, 128,   0   },   /* FL=0, FR=128, RL=128, RR=0    */
    /* BOUND  */ {   0,   0, 128, 128   },   /* FL/FR=0, RL/RR=128            */
    /* GALLOP */ {   0,  64, 192, 128   },   /* sequence offsets              */
};

/* Per-phase servo command target deltas (relative to neutral PWM 0).
 * Indexed [phase][joint_id]. Hand-tuned mk1 values; production firmware
 * replaces with closed-form IK from leg_kinematics.c.                   */
static const int16_t SERVO_TARGET_DELTA[GAIT_PHASES][JOINTS_PER_LEG] = {
    /* phase                 hip_yaw  hip_pitch  knee     */
    /* GAIT_PHASE_STANCE */ {     0,    -2000,   -3000  },
    /* GAIT_PHASE_SWING  */ {     0,     2500,    4000  },
    /* GAIT_PHASE_PUSH   */ {     0,    -3500,   -1500  },
    /* GAIT_PHASE_LAND   */ {     0,     1000,   -1000  },
};

uint32_t dog_joint_index(uint8_t leg, uint8_t joint) {
    return (uint32_t)leg * JOINTS_PER_LEG + (uint32_t)joint;
}

uint8_t dog_phase_at(uint32_t cycle_phase_q256, uint8_t leg) {
    /* Each gait phase occupies 1/4 of the cycle wheel = 64 Q256 units.   */
    uint32_t local = (cycle_phase_q256 + LEG_PHASE_OFFSET_Q256[dog_state.pattern][leg]) & 0xFFu;
    return (uint8_t)(local >> 6);   /* 0..3, exactly τ(6)=4 buckets       */
}

int16_t dog_servo_for(uint8_t leg, uint8_t joint, uint8_t phase) {
    if (phase >= GAIT_PHASES || joint >= JOINTS_PER_LEG) {
        return 0;
    }
    int32_t cmd = SERVO_TARGET_DELTA[phase][joint];
    /* Mirror left/right pair so legs on opposite sides drive symmetrically. */
    if (leg == LEG_FR || leg == LEG_RR) {
        cmd = -cmd;
    }
    if (cmd > SERVO_PWM_MAX) cmd = SERVO_PWM_MAX;
    if (cmd < SERVO_PWM_MIN) cmd = SERVO_PWM_MIN;
    return (int16_t)cmd;
}

void dog_gait_step(uint32_t dt_ms) {
    /* Cycle period — STAND: never advance, TROT: 800 ms full cycle.
     * Q256 increment per millisecond:
     *     TROT  inc = 256 / 800 ≈ 0.32  → use 256·dt/800 ≈ dt·8/25
     */
    static uint32_t cycle_q256 = 0;
    if (dog_state.pattern != GAIT_PATTERN_STAND) {
        uint32_t inc = (dt_ms * 256u) / 800u;
        cycle_q256 = (cycle_q256 + inc) & 0xFFu;
        /* full-cycle event → bump cycle counter */
        if (inc != 0 && cycle_q256 < inc) {
            dog_state.cycle_count++;
        }
    } else {
        cycle_q256 = 0;
    }

    for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) {
        uint8_t phase = dog_phase_at(cycle_q256, leg);
        dog_state.phase[leg] = phase;
        for (uint8_t j = 0; j < JOINTS_PER_LEG; j++) {
            uint32_t idx = dog_joint_index(leg, j);
            dog_state.servo_cmd[idx] = dog_servo_for(leg, j, phase);
        }
    }

    /* gait state machine transitions */
    switch (dog_state.state_id) {
    case DOG_STATE_BOOT:
        dog_state.state_id = DOG_STATE_CALIB;
        break;
    case DOG_STATE_CALIB:
        dog_state.state_id = DOG_STATE_STAND;
        break;
    case DOG_STATE_STAND:
        if (dog_state.pattern != GAIT_PATTERN_STAND) {
            dog_state.state_id = DOG_STATE_LOCOMOTE;
        }
        break;
    case DOG_STATE_LOCOMOTE:
        if (dog_state.pattern == GAIT_PATTERN_STAND) {
            dog_state.state_id = DOG_STATE_STAND;
        }
        break;
    case DOG_STATE_FAULT:
    case DOG_STATE_SHUTDOWN:
        /* terminal — host clears flags to recover */
        break;
    default:
        dog_state.state_id = DOG_STATE_BOOT;
        break;
    }
}
