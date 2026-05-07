/* dog_sim.c — host-side dog_robot firmware simulator.
 *
 * Builds the firmware src under SIM_BUILD on the host (POSIX C, no ARM)
 * and drives a synthetic main loop for N gait cycles. Emits one record
 * per gait wheel tick to stdout (text or JSON) so tests/test_firmware_sim.py
 * can assert against the gait phase trajectory.
 *
 * Pattern source: hexa-sscb sim concept (--mk=N flag) — here mapped onto
 * --gait=stand|trot|bound|gallop and the .roadmap.hexa_bot v1.3.0 plan
 * (BOUND + GALLOP are Mk.II per gait_sm.c GAIT_PATTERN_* enum).
 *
 * Usage:
 *   ./dog_sim --gait=stand           # 200 ms, expect all phases STANCE
 *   ./dog_sim --gait=trot --ms=2000  # 2 s of TROT, JSON one-line summary
 *   ./dog_sim --gait=bound --json
 *   ./dog_sim --gait=gallop --csv    # CSV gait wheel for plotting
 *   ./dog_sim --selftest             # internal coherence sweep
 *
 * Returns 0 on success, 1 on assertion failure.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../include/dog_robot.h"
#include "mock_arch.h"

/* dog_state lives in firmware src/main.c; sim links against it. */
extern struct dog_state dog_state;

/* Forward decls (defined in firmware src/, compiled under SIM_BUILD). */
void dog_system_init(void);
void dog_gait_step(uint32_t dt_ms);
void dog_servo_apply(void);
void dog_fault_check(void);

/* sim-only — defined in mock_hardware.c. */
void sim_reset_hardware(void);

static const char *GAIT_NAMES[] = {"stand", "trot", "bound", "gallop"};
static const char *PHASE_NAMES[] = {"STANCE", "SWING", "PUSH", "LAND"};
static const char *STATE_NAMES[] = {
    "BOOT", "CALIB", "STAND", "LOCOMOTE", "FAULT", "SHUTDOWN"
};

struct cli_opts {
    int  gait;       /* 0..3 */
    int  duration_ms;
    int  json;
    int  csv;
    int  selftest;
    int  verbose;
};

static int parse_gait(const char *s) {
    if (!s) return -1;
    for (int i = 0; i < 4; i++) {
        if (strcmp(s, GAIT_NAMES[i]) == 0) return i;
    }
    return -1;
}

static int parse_args(int argc, char **argv, struct cli_opts *o) {
    o->gait = GAIT_PATTERN_TROT;
    o->duration_ms = 1600;
    o->json = 0; o->csv = 0; o->selftest = 0; o->verbose = 0;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strncmp(a, "--gait=", 7) == 0) {
            int g = parse_gait(a + 7);
            if (g < 0) {
                fprintf(stderr, "unknown gait: %s (want stand|trot|bound|gallop)\n", a + 7);
                return 2;
            }
            o->gait = g;
        } else if (strncmp(a, "--ms=", 5) == 0) {
            o->duration_ms = atoi(a + 5);
        } else if (strcmp(a, "--json") == 0) {
            o->json = 1;
        } else if (strcmp(a, "--csv") == 0) {
            o->csv = 1;
        } else if (strcmp(a, "--selftest") == 0) {
            o->selftest = 1;
        } else if (strcmp(a, "--verbose") == 0 || strcmp(a, "-v") == 0) {
            o->verbose = 1;
        } else if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            printf(
                "dog_sim — host-side dog_robot firmware simulator\n"
                "\n"
                "options:\n"
                "  --gait=NAME    stand|trot|bound|gallop   (default trot)\n"
                "  --ms=N         simulation duration ms    (default 1600)\n"
                "  --json         one-line JSON summary\n"
                "  --csv          per-tick CSV (tick_ms,leg,phase,servo[3])\n"
                "  --selftest     run internal sweep (returns 0 on success)\n"
                "  --verbose      print state transitions\n"
                "  --help         this help\n"
            );
            exit(0);
        } else {
            fprintf(stderr, "unknown flag: %s (try --help)\n", a);
            return 2;
        }
    }
    return 0;
}

static void emit_csv_row(uint32_t t, uint8_t leg) {
    uint32_t i0 = dog_joint_index(leg, JOINT_HIP_YAW);
    uint32_t i1 = dog_joint_index(leg, JOINT_HIP_PITCH);
    uint32_t i2 = dog_joint_index(leg, JOINT_KNEE);
    printf("%u,%u,%s,%d,%d,%d\n",
           t, leg, PHASE_NAMES[dog_state.phase[leg]],
           dog_state.servo_cmd[i0],
           dog_state.servo_cmd[i1],
           dog_state.servo_cmd[i2]);
}

static int run_simulation(struct cli_opts *o) {
    sim_reset_hardware();
    memset(&dog_state, 0, sizeof dog_state);
    dog_state.state_id = DOG_STATE_BOOT;
    dog_state.pattern  = (uint8_t)o->gait;

    if (o->csv) {
        printf("tick_ms,leg,phase,servo_yaw,servo_pitch,servo_knee\n");
    }

    uint32_t step_ms = 5;
    uint32_t phase_count[GAIT_PHASES] = {0};

    for (uint32_t t = 0; t < (uint32_t)o->duration_ms; t += step_ms) {
        dog_gait_step(step_ms);
        dog_servo_apply();
        dog_fault_check();
        dog_state.tick_ms = t;
        if (o->csv) {
            for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) {
                emit_csv_row(t, leg);
            }
        }
        for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) {
            phase_count[dog_state.phase[leg]]++;
        }
    }

    if (o->json) {
        printf("{\"tool\":\"dog_sim\",\"schema\":\"hexa-bot/firmware/sim/v1\",");
        printf("\"gait\":\"%s\",\"duration_ms\":%d,",
               GAIT_NAMES[o->gait], o->duration_ms);
        printf("\"final_state\":\"%s\",\"cycle_count\":%u,",
               STATE_NAMES[dog_state.state_id], dog_state.cycle_count);
        printf("\"phase_distribution\":{\"STANCE\":%u,\"SWING\":%u,\"PUSH\":%u,\"LAND\":%u}",
               phase_count[0], phase_count[1], phase_count[2], phase_count[3]);
        printf(",\"flags\":%u}\n", dog_state.flags);
    } else if (!o->csv) {
        printf("dog_sim — gait=%s duration=%d ms\n", GAIT_NAMES[o->gait], o->duration_ms);
        printf("  final state    : %s\n", STATE_NAMES[dog_state.state_id]);
        printf("  cycles         : %u\n", dog_state.cycle_count);
        printf("  phase counts   : STANCE=%u SWING=%u PUSH=%u LAND=%u\n",
               phase_count[0], phase_count[1], phase_count[2], phase_count[3]);
        printf("  fault flags    : 0x%x\n", dog_state.flags);
    }

    if (dog_state.flags != 0) return 1;
    return 0;
}

static int run_selftest(void) {
    /* For each gait pattern, run 1.6 s and check:
     *   STAND  : all 4 legs report STANCE phase only, cycle_count == 0
     *   TROT   : all 4 phases visited, FL ↔ RR diagonal pair phase-aligned
     *   BOUND  : front pair (FL,FR) phase-aligned, rear pair (RL,RR) phase-aligned
     *   GALLOP : all 4 legs sample distinct phases at some t                 */
    int fails = 0;

    /* STAND */
    {
        sim_reset_hardware();
        memset(&dog_state, 0, sizeof dog_state);
        dog_state.state_id = DOG_STATE_BOOT;
        dog_state.pattern  = GAIT_PATTERN_STAND;
        for (uint32_t t = 0; t < 1600; t += 5) dog_gait_step(5);
        for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) {
            if (dog_state.phase[leg] != GAIT_PHASE_STANCE) {
                fprintf(stderr, "[FAIL] STAND: leg %u not in STANCE\n", leg);
                fails++;
            }
        }
        if (dog_state.cycle_count != 0) {
            fprintf(stderr, "[FAIL] STAND: cycle_count=%u (want 0)\n", dog_state.cycle_count);
            fails++;
        }
    }
    /* TROT — diagonal pair check at multiple t */
    {
        sim_reset_hardware();
        memset(&dog_state, 0, sizeof dog_state);
        dog_state.state_id = DOG_STATE_BOOT;
        dog_state.pattern  = GAIT_PATTERN_TROT;
        int diag_aligned = 1;
        int phases_seen[GAIT_PHASES] = {0};
        for (uint32_t t = 0; t < 1600; t += 5) {
            dog_gait_step(5);
            phases_seen[dog_state.phase[LEG_FL]]++;
            if (dog_state.phase[LEG_FL] != dog_state.phase[LEG_RR]) diag_aligned = 0;
            if (dog_state.phase[LEG_FR] != dog_state.phase[LEG_RL]) diag_aligned = 0;
        }
        if (!diag_aligned) {
            fprintf(stderr, "[FAIL] TROT: diagonal pairs not phase-aligned\n");
            fails++;
        }
        for (int p = 0; p < GAIT_PHASES; p++) {
            if (phases_seen[p] == 0) {
                fprintf(stderr, "[FAIL] TROT: never visited phase %s\n", PHASE_NAMES[p]);
                fails++;
            }
        }
    }
    /* BOUND — front/rear pair alignment */
    {
        sim_reset_hardware();
        memset(&dog_state, 0, sizeof dog_state);
        dog_state.state_id = DOG_STATE_BOOT;
        dog_state.pattern  = GAIT_PATTERN_BOUND;
        int aligned = 1;
        for (uint32_t t = 0; t < 1600; t += 5) {
            dog_gait_step(5);
            if (dog_state.phase[LEG_FL] != dog_state.phase[LEG_FR]) aligned = 0;
            if (dog_state.phase[LEG_RL] != dog_state.phase[LEG_RR]) aligned = 0;
        }
        if (!aligned) {
            fprintf(stderr, "[FAIL] BOUND: front pair or rear pair not aligned\n");
            fails++;
        }
    }
    /* GALLOP — distinct phases observed across legs */
    {
        sim_reset_hardware();
        memset(&dog_state, 0, sizeof dog_state);
        dog_state.state_id = DOG_STATE_BOOT;
        dog_state.pattern  = GAIT_PATTERN_GALLOP;
        int distinct_event = 0;
        for (uint32_t t = 0; t < 1600; t += 5) {
            dog_gait_step(5);
            int p[4] = {0,0,0,0};
            for (uint8_t leg = 0; leg < 4; leg++) p[dog_state.phase[leg]]++;
            int unique = 0;
            for (int k = 0; k < 4; k++) if (p[k] > 0) unique++;
            if (unique == 4) { distinct_event = 1; break; }
        }
        if (!distinct_event) {
            fprintf(stderr, "[FAIL] GALLOP: never had 4 distinct phases simultaneously\n");
            fails++;
        }
    }

    if (fails == 0) {
        printf("__DOG_SIM_SELFTEST__ PASS (4/4 gait patterns coherent)\n");
        return 0;
    }
    printf("__DOG_SIM_SELFTEST__ FAIL (%d failure(s))\n", fails);
    return 1;
}

int main(int argc, char **argv) {
    struct cli_opts o;
    int rc = parse_args(argc, argv, &o);
    if (rc) return rc;
    if (o.selftest) return run_selftest();
    return run_simulation(&o);
}
