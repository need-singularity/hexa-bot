/* dog_robot.h — quadruped reference firmware state + macros.
 *
 * Source: dog_robot/dog-robot-test.md (the spec) + .roadmap.hexa_bot §A.1
 * (n=6 lattice projection: τ(6)=4 gait phases · σ(6)=12 joints · 4 legs).
 *
 * The numerics are the contractual invariant; if any of GAIT_PHASES /
 * DOG_LEG_COUNT / DOG_JOINTS_TOTAL changes the lattice has been violated
 * and verify/n6_arithmetic.py + verify/firmware_sync.py will catch it.
 *
 * Pattern source: hexa-sscb/module/firmware/include/sscb.h.
 */
#ifndef DOG_ROBOT_H
#define DOG_ROBOT_H

#include <stdint.h>

/* ── n=6 invariant projection ─────────────────────────────────────────── */
#define GAIT_PHASES         4    /* τ(6)=4 — stance / swing / push / land */
#define DOG_LEG_COUNT       4    /* quadruped (verb-bound, immutable mk1) */
#define JOINTS_PER_LEG      3    /* hip-yaw + hip-pitch + knee-pitch     */
#define DOG_JOINTS_TOTAL    12   /* σ(6)=12 = LEG_COUNT × JOINTS_PER_LEG */

/* ── gait phase enum (τ(6)=4) ─────────────────────────────────────────── */
enum gait_phase {
    GAIT_PHASE_STANCE = 0,    /* foot loaded, supports body              */
    GAIT_PHASE_SWING  = 1,    /* foot lifted, advancing                  */
    GAIT_PHASE_PUSH   = 2,    /* drive-off impulse                       */
    GAIT_PHASE_LAND   = 3,    /* touchdown + impact absorption           */
};

/* ── gait pattern enum — mk1 supports stand + trot only.
 *    Mk.II adds bound + gallop (see .roadmap §A.2 v1.3.0).             */
enum gait_pattern {
    GAIT_PATTERN_STAND  = 0,
    GAIT_PATTERN_TROT   = 1,
    GAIT_PATTERN_BOUND  = 2,   /* Mk.II */
    GAIT_PATTERN_GALLOP = 3,   /* Mk.II */
};

/* ── leg id (FL/FR/RL/RR — front/rear × left/right) ─── */
enum leg_id {
    LEG_FL = 0,   /* front left  */
    LEG_FR = 1,   /* front right */
    LEG_RL = 2,   /* rear  left  */
    LEG_RR = 3,   /* rear  right */
};

/* ── joint id within a leg ─── */
enum joint_id {
    JOINT_HIP_YAW   = 0,
    JOINT_HIP_PITCH = 1,
    JOINT_KNEE      = 2,
};

/* ── servo command range (mk1: int16 PWM count, ±18000) ─── */
#define SERVO_PWM_MIN       (-18000)
#define SERVO_PWM_MAX       ( 18000)

/* ── fault flags (bitfield) ─── */
#define DOG_FLT_SERVO_TIMEOUT    (1u << 0)
#define DOG_FLT_OVERCURRENT      (1u << 1)
#define DOG_FLT_IMU_DROPOUT      (1u << 2)
#define DOG_FLT_FOOT_LOSS        (1u << 3)
#define DOG_FLT_OVERTEMPERATURE  (1u << 4)

/* ── state machine ids ─── */
enum dog_state_id {
    DOG_STATE_BOOT     = 0,
    DOG_STATE_CALIB    = 1,
    DOG_STATE_STAND    = 2,
    DOG_STATE_LOCOMOTE = 3,
    DOG_STATE_FAULT    = 4,
    DOG_STATE_SHUTDOWN = 5,
};

/* ── single global state struct (no heap) ─── */
struct dog_state {
    uint32_t flags;                            /* bitfield of DOG_FLT_*       */
    uint8_t  state_id;                         /* enum dog_state_id            */
    uint8_t  pattern;                          /* enum gait_pattern            */
    uint8_t  phase[DOG_LEG_COUNT];             /* per-leg gait phase           */
    uint16_t cycle_count;                      /* full-gait cycles since boot  */
    int16_t  servo_cmd[DOG_JOINTS_TOTAL];      /* PWM counts, σ(6)=12 channels */
    int16_t  imu_pitch_mdeg;                   /* signed milli-degrees         */
    int16_t  imu_roll_mdeg;
    uint32_t tick_ms;                          /* monotonic ms since boot      */
};

extern struct dog_state dog_state;

/* ── firmware entry points ─── */
void dog_system_init(void);
void dog_main_loop(void);
void dog_gait_step(uint32_t dt_ms);
void dog_servo_apply(void);
void dog_fault_check(void);

/* ── helpers (declared so main.c + sim share definitions) ─── */
uint8_t  dog_phase_at(uint32_t cycle_phase_q256, uint8_t leg);
int16_t  dog_servo_for(uint8_t leg, uint8_t joint, uint8_t phase);
uint32_t dog_joint_index(uint8_t leg, uint8_t joint);

/* ── compile-time guard for the n=6 lattice ─── */
_Static_assert(DOG_JOINTS_TOTAL == DOG_LEG_COUNT * JOINTS_PER_LEG,
               "n=6 lattice violation: σ(6)=12 must equal 4 legs × 3 joints");
_Static_assert(GAIT_PHASES == 4,
               "n=6 lattice violation: τ(6)=4 gait phases");

#endif /* DOG_ROBOT_H */
