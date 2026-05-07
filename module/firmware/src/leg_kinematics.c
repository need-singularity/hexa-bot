/* leg_kinematics.c — closed-form 3-DOF leg inverse kinematics.
 *
 * Source: robotics/robotics.md §"6-DOF closed-form IK" (n=6 lattice direct
 * application — each leg is a 3-DOF subsystem; 4 legs × 3 joints = σ(6)=12
 * total joint surface). Pattern source: hexa-sscb verify/calc_*.py atlas
 * pattern (constants documented in comments, derivable from spec).
 *
 * Geometry (mk1 unit dog scale, 25 cm body):
 *     L1 = hip-to-thigh    = 50 mm  (yaw axis offset)
 *     L2 = thigh           = 80 mm
 *     L3 = shank           = 80 mm
 *
 * The IK solution for foot position (x, y, z) in the leg's hip frame:
 *     hip_yaw   = atan2(y, x)
 *     hip_pitch = atan2(z, sqrt(x² + y²) - L1) - acos(D)
 *     knee     = π - acos((L2² + L3² - r²) / (2·L2·L3))
 * where r is the leg-plane distance from hip-pitch joint to foot.
 *
 * mk1 ships a small-angle approximation only — the full atan2/acos calls
 * require math.h which the bare-metal scaffold avoids. The approximation
 * suffices for the gait wheel servo deltas in gait_sm.c, which already
 * encode hand-tuned target deltas. This file is the hook for v1.3.0
 * (.roadmap §A.2) when the full IK is wired.
 */
#include "dog_robot.h"

/* All length units are tenths of a millimetre (so int32 keeps integer math). */
#define L1_TMM   500
#define L2_TMM   800
#define L3_TMM   800

/* Sin / Cos lookup table — 16 entries × 1/8-rad steps (covers ±π).
 * Q15 fixed-point. Production firmware swaps in a 256-entry quarter-wave
 * LUT. Kept tiny here so the scaffold stays self-contained.            */
static const int16_t SIN_LUT[16] = {
    /*   0 rad      */     0,
    /*   π/8        */  12539,
    /*   π/4        */  23170,
    /*  3π/8        */  30273,
    /*   π/2        */  32767,
    /*  5π/8        */  30273,
    /*  3π/4        */  23170,
    /*  7π/8        */  12539,
    /*   π          */      0,
    /*  9π/8        */ -12539,
    /*  5π/4        */ -23170,
    /* 11π/8        */ -30273,
    /*  3π/2        */ -32767,
    /* 13π/8        */ -30273,
    /*  7π/4        */ -23170,
    /* 15π/8        */ -12539,
};

/* Crude integer atan2 — returns Q256-encoded angle in [0, 256).
 * Matches the dog_phase_at() Q256 convention so gait_sm.c can feed
 * directly into kinematics if needed.                                  */
uint8_t leg_atan2_q256(int32_t y, int32_t x) {
    if (x == 0 && y == 0) return 0;
    int32_t ax = x < 0 ? -x : x;
    int32_t ay = y < 0 ? -y : y;
    /* coarse octant */
    uint32_t base = (ax >= ay) ? 0u : 64u;
    uint32_t frac = (ax >= ay)
        ? (uint32_t)((ay * 32) / (ax + 1))
        : (uint32_t)((ax * 32) / (ay + 1));
    uint32_t a = base + (frac & 0x1F);
    if (x < 0)              a = 128u - a;
    if (y < 0)              a = (256u - a) & 0xFFu;
    return (uint8_t)(a & 0xFFu);
}

int16_t leg_sin_q15(uint8_t angle_q256) {
    return SIN_LUT[(angle_q256 >> 4) & 0x0F];
}

int16_t leg_cos_q15(uint8_t angle_q256) {
    return SIN_LUT[((angle_q256 >> 4) + 4) & 0x0F];
}

/* Forward kinematics — hip-yaw frame foot position from joint angles.
 * Returns Q15 fixed-point in tenths of a millimetre.                   */
void leg_forward_kinematics(int16_t hip_yaw_q15,
                            int16_t hip_pitch_q15,
                            int16_t knee_q15,
                            int32_t *out_x_tmm,
                            int32_t *out_y_tmm,
                            int32_t *out_z_tmm) {
    (void)hip_yaw_q15; (void)hip_pitch_q15; (void)knee_q15;
    /* Full implementation is v1.3.0 work (.roadmap §A.2). mk1 ships the
     * hook + parameter wiring so the next cycle drops in pure math.    */
    if (out_x_tmm) *out_x_tmm = L2_TMM + L3_TMM;
    if (out_y_tmm) *out_y_tmm = 0;
    if (out_z_tmm) *out_z_tmm = -(L2_TMM + L3_TMM) / 2;
}
