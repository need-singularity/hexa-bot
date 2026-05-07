/* mock_arch.h — host-build register layout for dog_robot firmware sim.
 *
 * Pattern source: hexa-sscb pattern of replacing MMIO-based register
 * structs with plain memory in mock_hardware.c so the firmware sources
 * compile + link on macOS/Linux without an arm-none-eabi toolchain.
 *
 * Activated by `-DSIM_BUILD` in the sim Makefile. Layout matches the real
 * STM32F429 register positions so any code that touches TIM1->CCR1 etc.
 * works identically against the mock.
 */
#ifndef DOG_FIRMWARE_SIM_MOCK_ARCH_H
#define DOG_FIRMWARE_SIM_MOCK_ARCH_H

#include <stdint.h>

/* ---- TIM1 / TIM2 (advanced timer) ---- */
typedef struct {
    volatile uint32_t CR1, CR2, SMCR, DIER, SR, EGR;
    volatile uint32_t CCMR1, CCMR2, CCER;
    volatile uint32_t CNT, PSC, ARR, RCR;
    volatile uint32_t CCR1, CCR2, CCR3, CCR4;
    volatile uint32_t BDTR, DCR, DMAR;
} TIM_TypeDef;
extern TIM_TypeDef _mock_TIM1;
extern TIM_TypeDef _mock_TIM2;
#define TIM1 (&_mock_TIM1)
#define TIM2 (&_mock_TIM2)

#define TIM_SR_UIF      (1u << 0)
#define TIM_EGR_UG      (1u << 0)

/* ---- DWT cycle counter ---- */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t CYCCNT;
} DWT_Type;
extern DWT_Type _mock_DWT;
#define DWT (&_mock_DWT)
#define DWT_CTRL_CYCCNTENA (1u << 0)

/* ---- DEMCR ---- */
extern volatile uint32_t _mock_DEMCR;
#define DEMCR _mock_DEMCR
#define DEMCR_TRCENA    (1u << 24)

/* ---- IRQ ids ---- */
typedef enum {
    NonMaskableInt_IRQn      = -14,
    HardFault_IRQn           = -13,
    SysTick_IRQn             = -1,
    TIM2_IRQn                = 28,
} IRQn_Type;

static inline void NVIC_EnableIRQ(IRQn_Type irq)         { (void)irq; }
static inline void NVIC_SetPriority(IRQn_Type irq, uint32_t p) { (void)irq; (void)p; }

/* ---- RCC mock ---- */
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
extern RCC_TypeDef _mock_RCC;
#define RCC (&_mock_RCC)

#define RCC_CR_HSEON    (1u << 16)
#define RCC_CR_HSERDY   (1u << 17)
#define RCC_CR_PLLON    (1u << 24)
#define RCC_CR_PLLRDY   (1u << 25)
#define RCC_CFGR_SW_PLL (0x2u << 0)
#define RCC_CFGR_SWS    (0x3u << 2)
#define RCC_CFGR_SWS_PLL (0x2u << 2)

/* Sim-only helpers (defined in mock_hardware.c). */
void sim_reset_hardware(void);
void sim_advance_cycles(uint32_t cycles);
void sim_inject_imu(int16_t pitch_mdeg, int16_t roll_mdeg);

#endif /* DOG_FIRMWARE_SIM_MOCK_ARCH_H */
