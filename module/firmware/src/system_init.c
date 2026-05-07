/* system_init.c — clock tree + NVIC priorities for dog_robot mk1.
 *
 * HSE 8 MHz × PLL → SYSCLK 180 MHz, AHB 180, APB1 45, APB2 90.
 * Pattern source: hexa-sscb/module/firmware/src/system_init.c.
 */
#include "stm32f4xx_min.h"
#include "dog_robot.h"

#if !defined(SIM_BUILD)

void dog_system_init(void) {
    /* Enable HSE oscillator and wait for stable. */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) { /* spin */ }

    /* PLL: HSE 8 MHz / M=4 = 2 MHz × N=180 = 360 MHz / P=2 = 180 MHz. */
    RCC->PLLCFGR = (1u << 22)         /* PLLSRC = HSE */
                 | (4u   << 0)        /* PLLM   = 4   */
                 | (180u << 6)        /* PLLN   = 180 */
                 | (0u   << 16)       /* PLLP   = /2  */
                 | (7u   << 24);      /* PLLQ   = /7  (USB 48 MHz, unused) */

    RCC->CFGR = (5u << 10)            /* PPRE1 = 0b101 → /4 */
              | (4u << 13);           /* PPRE2 = 0b100 → /2 */

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) { /* spin */ }

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { /* spin */ }

    /* TIM2 = 50 Hz servo PWM update tick. NVIC priority above gait step. */
    NVIC_SetPriority(TIM2_IRQn,  1);
    NVIC_SetPriority(SysTick_IRQn, 14);
    NVIC_EnableIRQ(TIM2_IRQn);
}

#endif /* !SIM_BUILD */
