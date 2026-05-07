/* test_gait_sm.c — unit tests for gait_sm.c.
 *
 * Each test exercises one concrete invariant from dog_robot/dog-robot-test.md
 * and the .roadmap.hexa_bot v1.3.0 plan. Pure C, builds against the SIM_BUILD
 * variant of the firmware so it runs under valgrind / lldb on macOS.
 *
 * No test framework — each TEST() returns 0 on pass, 1 on fail; main()
 * counts and reports. Pattern: classic xunit-style with explicit asserts.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../include/dog_robot.h"
#include "../sim/mock_arch.h"

extern struct dog_state dog_state;
void dog_gait_step(uint32_t dt_ms);
void dog_servo_apply(void);
void sim_reset_hardware(void);

static int g_fails = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "[FAIL] %s:%d %s\n", __FILE__, __LINE__, msg); g_fails++; } \
} while (0)

#define BEGIN_TEST(name) static int name(void) { int __pre = g_fails; printf("  [test] %s\n", #name);
#define END_TEST         return g_fails - __pre; }

static void reset_state(uint8_t pattern) {
    sim_reset_hardware();
    memset(&dog_state, 0, sizeof dog_state);
    dog_state.state_id = DOG_STATE_BOOT;
    dog_state.pattern  = pattern;
}


BEGIN_TEST(test_lattice_macros_consistent)
    TEST_ASSERT(GAIT_PHASES == 4, "τ(6)=4 — 4 gait phases");
    TEST_ASSERT(DOG_LEG_COUNT == 4, "quadruped — 4 legs");
    TEST_ASSERT(JOINTS_PER_LEG == 3, "3-DOF leg");
    TEST_ASSERT(DOG_JOINTS_TOTAL == DOG_LEG_COUNT * JOINTS_PER_LEG,
                "σ(6)=12 = 4 × 3 lattice closure");
END_TEST

BEGIN_TEST(test_stand_holds_stance)
    reset_state(GAIT_PATTERN_STAND);
    /* Run 800 ms — 1 nominal cycle period; STAND should never advance. */
    for (uint32_t t = 0; t < 800; t += 5) dog_gait_step(5);
    TEST_ASSERT(dog_state.cycle_count == 0, "STAND must not advance cycle counter");
    for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) {
        TEST_ASSERT(dog_state.phase[leg] == GAIT_PHASE_STANCE,
                    "STAND keeps every leg in STANCE");
    }
END_TEST

BEGIN_TEST(test_trot_diagonal_pairs_aligned)
    reset_state(GAIT_PATTERN_TROT);
    int diag = 1;
    for (uint32_t t = 0; t < 1600; t += 5) {
        dog_gait_step(5);
        if (dog_state.phase[LEG_FL] != dog_state.phase[LEG_RR]) diag = 0;
        if (dog_state.phase[LEG_FR] != dog_state.phase[LEG_RL]) diag = 0;
    }
    TEST_ASSERT(diag, "TROT — diagonal pairs (FL↔RR, FR↔RL) phase-aligned");
END_TEST

BEGIN_TEST(test_trot_visits_all_phases)
    reset_state(GAIT_PATTERN_TROT);
    int seen[GAIT_PHASES] = {0};
    for (uint32_t t = 0; t < 1600; t += 5) {
        dog_gait_step(5);
        for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) seen[dog_state.phase[leg]]++;
    }
    for (int p = 0; p < GAIT_PHASES; p++) {
        TEST_ASSERT(seen[p] > 0, "TROT — each phase visited at least once");
    }
END_TEST

BEGIN_TEST(test_bound_front_rear_symmetry)
    reset_state(GAIT_PATTERN_BOUND);
    int sym = 1;
    for (uint32_t t = 0; t < 1600; t += 5) {
        dog_gait_step(5);
        if (dog_state.phase[LEG_FL] != dog_state.phase[LEG_FR]) sym = 0;
        if (dog_state.phase[LEG_RL] != dog_state.phase[LEG_RR]) sym = 0;
    }
    TEST_ASSERT(sym, "BOUND — front pair phase-aligned, rear pair phase-aligned");
END_TEST

BEGIN_TEST(test_servo_cmd_within_range)
    reset_state(GAIT_PATTERN_TROT);
    int ok = 1;
    for (uint32_t t = 0; t < 1600; t += 5) {
        dog_gait_step(5);
        for (int i = 0; i < DOG_JOINTS_TOTAL; i++) {
            int16_t v = dog_state.servo_cmd[i];
            if (v < SERVO_PWM_MIN || v > SERVO_PWM_MAX) ok = 0;
        }
    }
    TEST_ASSERT(ok, "every servo command stays within ±SERVO_PWM_MAX");
END_TEST

BEGIN_TEST(test_state_transition_boot_to_locomote)
    reset_state(GAIT_PATTERN_TROT);
    TEST_ASSERT(dog_state.state_id == DOG_STATE_BOOT, "starts in BOOT");
    dog_gait_step(5);  /* BOOT → CALIB */
    TEST_ASSERT(dog_state.state_id == DOG_STATE_CALIB, "first step → CALIB");
    dog_gait_step(5);  /* CALIB → STAND */
    TEST_ASSERT(dog_state.state_id == DOG_STATE_STAND, "second step → STAND");
    dog_gait_step(5);  /* STAND with TROT pattern → LOCOMOTE */
    TEST_ASSERT(dog_state.state_id == DOG_STATE_LOCOMOTE, "non-stand pattern → LOCOMOTE");
END_TEST

BEGIN_TEST(test_joint_index_is_dense)
    /* dog_joint_index must be a bijection over [0, σ(6)=12). */
    int seen[DOG_JOINTS_TOTAL] = {0};
    for (uint8_t leg = 0; leg < DOG_LEG_COUNT; leg++) {
        for (uint8_t j = 0; j < JOINTS_PER_LEG; j++) {
            uint32_t idx = dog_joint_index(leg, j);
            TEST_ASSERT(idx < (uint32_t)DOG_JOINTS_TOTAL, "index in range");
            seen[idx]++;
        }
    }
    int dense = 1;
    for (int k = 0; k < DOG_JOINTS_TOTAL; k++) if (seen[k] != 1) dense = 0;
    TEST_ASSERT(dense, "every index hit exactly once (12-element bijection)");
END_TEST


typedef struct { const char *name; int (*fn)(void); } test_t;

int main(void) {
    test_t tests[] = {
        {"test_lattice_macros_consistent",     test_lattice_macros_consistent},
        {"test_stand_holds_stance",            test_stand_holds_stance},
        {"test_trot_diagonal_pairs_aligned",   test_trot_diagonal_pairs_aligned},
        {"test_trot_visits_all_phases",        test_trot_visits_all_phases},
        {"test_bound_front_rear_symmetry",     test_bound_front_rear_symmetry},
        {"test_servo_cmd_within_range",        test_servo_cmd_within_range},
        {"test_state_transition_boot_to_locomote", test_state_transition_boot_to_locomote},
        {"test_joint_index_is_dense",          test_joint_index_is_dense},
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int total_fails = 0;
    printf("== test_gait_sm — %d test(s) ==\n", n);
    for (int i = 0; i < n; i++) {
        int f = tests[i].fn();
        total_fails += f;
        printf("  %s : %s\n", tests[i].name, f == 0 ? "PASS" : "FAIL");
    }
    printf("================================\n");
    printf("  %d/%d PASS\n", n - (total_fails > 0 ? 1 : 0), n);
    /* count per-test by recomputing (rough — total_fails is sum across tests) */
    if (total_fails > 0) {
        printf("  total assertion fails: %d\n", total_fails);
        return 1;
    }
    return 0;
}
