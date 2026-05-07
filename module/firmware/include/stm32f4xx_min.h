/* stm32f4xx_min.h — minimal CMSIS-style register stub for dog_robot fw.
 *
 * Pattern source: hexa-sscb/module/firmware/include/stm32f4xx_min.h.
 *
 * NOT a redistribution of ST's Cube SDK headers. Defines only the registers
 * the dog_robot reference scaffold touches. Production builds replace this
 * header with the vendor stm32f4xx.h via `-I /path/to/CMSIS/...` and
 * `-DUSE_VENDOR_CMSIS`.
 *
 * Three build modes:
 *   default  — bare ARM target, register-mapped MMIO (this file's main body)
 *   USE_VENDOR_CMSIS — pull in real CMSIS device header
 *   SIM_BUILD — replace MMIO with mock memory (for host-side simulation)
 *
 * Ref: STMicroelectronics RM0090 Rev 19.
 */
#ifndef STM32F4XX_MIN_H
#define STM32F4XX_MIN_H

#include <stdint.h>

#if defined(SIM_BUILD)
/* Host-build mode: registers collapse onto plain memory in mock_hardware.c.
 * Layout below is identical so fault-handler / servo-driver code links
 * untouched on Linux/macOS host. */
#include "../sim/mock_arch.h"

#elif defined(USE_VENDOR_CMSIS)
#include "stm32f4xx.h"

#else  /* bare-metal ARM mk1 default */

/* ---- Memory map (RM0090 Table 1) ------------------------------------- */
#define PERIPH_BASE     0x40000000UL
#define APB1_BASE       (PERIPH_BASE + 0x00000000UL)
#define APB2_BASE       (PERIPH_BASE + 0x00010000UL)
#define AHB1_BASE       (PERIPH_BASE + 0x00020000UL)

/* ---- TIM1 (advanced timer, APB2) — RM0090 §17 ------------------------ */
typedef struct {
    volatile uint32_t CR1, CR2, SMCR, DIER, SR, EGR;
    volatile uint32_t CCMR1, CCMR2, CCER;
    volatile uint32_t CNT, PSC, ARR, RCR;
    volatile uint32_t CCR1, CCR2, CCR3, CCR4;
    volatile uint32_t BDTR, DCR, DMAR;
} TIM_TypeDef;
#define TIM1            ((TIM_TypeDef *)(APB2_BASE + 0x0000))
#define TIM2            ((TIM_TypeDef *)(APB1_BASE + 0x0000))

#define TIM_SR_UIF      (1u << 0)
#define TIM_EGR_UG      (1u << 0)

/* ---- DWT (cycle counter, Cortex-M Core Debug) ------------------------ */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t CYCCNT;
} DWT_Type;
#define DWT             ((DWT_Type *)0xE0001000UL)
#define DWT_CTRL_CYCCNTENA (1u << 0)

#define DEMCR           (*(volatile uint32_t *)0xE000EDFCUL)
#define DEMCR_TRCENA    (1u << 24)

/* ---- IRQ vector numbers (subset; RM0090 Table 62) ------------------- */
typedef enum {
    NonMaskableInt_IRQn      = -14,
    HardFault_IRQn           = -13,
    SysTick_IRQn             = -1,
    TIM2_IRQn                = 28,
} IRQn_Type;

/* ---- NVIC ---- */
typedef struct {
    volatile uint32_t ISER[8];
    uint32_t          _r0[24];
    volatile uint32_t ICER[8];
    uint32_t          _r1[24];
    volatile uint32_t ISPR[8];
    uint32_t          _r2[24];
    volatile uint32_t ICPR[8];
    uint32_t          _r3[24];
    volatile uint32_t IABR[8];
    uint32_t          _r4[56];
    volatile uint8_t  IP[240];
} NVIC_Type;
#define NVIC            ((NVIC_Type *)0xE000E100UL)

static inline void NVIC_EnableIRQ(IRQn_Type irq) {
    if ((int)irq >= 0)
        NVIC->ISER[((uint32_t)irq) >> 5] = (1u << (((uint32_t)irq) & 0x1F));
}
static inline void NVIC_SetPriority(IRQn_Type irq, uint32_t prio) {
    if ((int)irq >= 0)
        NVIC->IP[(uint32_t)irq] = (uint8_t)((prio & 0x0F) << 4);
}

/* ---- RCC (clock control, AHB1) — RM0090 §6 -------------------------- */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR, AHB2RSTR, AHB3RSTR; uint32_t _r0;
    volatile uint32_t APB1RSTR, APB2RSTR;          uint32_t _r1[2];
    volatile uint32_t AHB1ENR, AHB2ENR, AHB3ENR;   uint32_t _r2;
    volatile uint32_t APB1ENR, APB2ENR;
} RCC_TypeDef;
#define RCC             ((RCC_TypeDef *)(AHB1_BASE + 0x3800))

#define RCC_CR_HSEON    (1u << 16)
#define RCC_CR_HSERDY   (1u << 17)
#define RCC_CR_PLLON    (1u << 24)
#define RCC_CR_PLLRDY   (1u << 25)
#define RCC_CFGR_SW_PLL (0x2u << 0)
#define RCC_CFGR_SWS    (0x3u << 2)
#define RCC_CFGR_SWS_PLL (0x2u << 2)

#endif  /* SIM_BUILD / USE_VENDOR_CMSIS / fallback */

/* ---- ISR prototype attribute (used by startup vector table) ---------- */
#define ISR_HANDLER

#if !defined(SIM_BUILD)
static inline void dwt_cycle_counter_enable(void) {
    DEMCR |= DEMCR_TRCENA;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA;
    DWT->CYCCNT = 0;
}
#else
/* Sim provides this in mock_hardware.c so subsystems link to a real symbol. */
void dwt_cycle_counter_enable(void);
#endif

#endif /* STM32F4XX_MIN_H */
