/* mock_hardware.c — backing storage for the mock STM32 register layout.
 *
 * The mock is a plain memory image — no atomics, no spin loops, no IRQ
 * stack. The simulator drives time forward explicitly via
 * sim_advance_cycles() and sim_reset_hardware() so the C code can be
 * single-stepped under valgrind / lldb on macOS or Linux.
 *
 * Production firmware re-routes via the real stm32f4xx.h CMSIS header
 * (build with -DUSE_VENDOR_CMSIS) which makes this whole TU vanish.
 */
#include <stddef.h>
#include "mock_arch.h"
#include "stm32f4xx_min.h"
#include "dog_robot.h"

TIM_TypeDef    _mock_TIM1;
TIM_TypeDef    _mock_TIM2;
DWT_Type       _mock_DWT;
volatile uint32_t _mock_DEMCR;
RCC_TypeDef    _mock_RCC;

void sim_reset_hardware(void) {
    /* Bring all mocks back to reset state — all bits 0. */
    uint8_t *p = (uint8_t *)&_mock_TIM1; for (size_t i = 0; i < sizeof _mock_TIM1; i++) p[i] = 0;
    p = (uint8_t *)&_mock_TIM2;          for (size_t i = 0; i < sizeof _mock_TIM2; i++) p[i] = 0;
    p = (uint8_t *)&_mock_DWT;           for (size_t i = 0; i < sizeof _mock_DWT;  i++) p[i] = 0;
    p = (uint8_t *)&_mock_RCC;           for (size_t i = 0; i < sizeof _mock_RCC;  i++) p[i] = 0;
    _mock_DEMCR = 0;

    /* Pre-set "ready" flags so dog_system_init() spin loops exit. */
    _mock_RCC.CR   = RCC_CR_HSERDY | RCC_CR_PLLRDY;
    _mock_RCC.CFGR = RCC_CFGR_SWS_PLL;
}

void sim_advance_cycles(uint32_t cycles) {
    _mock_DWT.CYCCNT += cycles;
}

void sim_inject_imu(int16_t pitch_mdeg, int16_t roll_mdeg) {
    extern struct dog_state dog_state;
    dog_state.imu_pitch_mdeg = pitch_mdeg;
    dog_state.imu_roll_mdeg  = roll_mdeg;
}

/* SIM-mode dwt_cycle_counter_enable — declared in stm32f4xx_min.h sim
 * branch. Ensures only one definition exists across the host build. */
void dwt_cycle_counter_enable(void) {
    _mock_DEMCR |= DEMCR_TRCENA;
    _mock_DWT.CTRL |= DWT_CTRL_CYCCNTENA;
    _mock_DWT.CYCCNT = 0;
}

/* SIM stand-in for dog_system_init — sim Makefile keeps system_init.c
 * out of the host build (it's gated #if !defined(SIM_BUILD)). We still
 * need a host-callable initializer so sscb_sim equivalent gets one. */
void dog_system_init(void) {
    sim_reset_hardware();
}
