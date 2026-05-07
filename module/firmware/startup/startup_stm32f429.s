/* startup_stm32f429.s — minimal Cortex-M4 vector table + Reset_Handler.
 *
 * Pattern source: hexa-sscb/module/firmware/startup/startup_stm32f429.s.
 * Targets STM32F429ZI; modify if porting to a different MCU.
 */
    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

    .extern main
    .extern _estack
    .extern _sidata
    .extern _sdata
    .extern _edata
    .extern _sbss
    .extern _ebss

    .section .isr_vector,"a",%progbits
    .type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    .text
    .thumb_func
    .global Reset_Handler
Reset_Handler:
    /* Copy .data from flash to RAM */
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_sidata
1:
    cmp   r0, r1
    bge   2f
    ldr   r3, [r2], #4
    str   r3, [r0], #4
    b     1b
2:
    /* Zero .bss */
    ldr   r0, =_sbss
    ldr   r1, =_ebss
3:
    cmp   r0, r1
    bge   4f
    movs  r3, #0
    str   r3, [r0], #4
    b     3b
4:
    bl    main
5:  b     5b

    .thumb_func
NMI_Handler:        b NMI_Handler
    .thumb_func
HardFault_Handler:  b HardFault_Handler
    .thumb_func
MemManage_Handler:  b MemManage_Handler
    .thumb_func
BusFault_Handler:   b BusFault_Handler
    .thumb_func
UsageFault_Handler: b UsageFault_Handler
    .thumb_func
SVC_Handler:        b SVC_Handler
    .thumb_func
DebugMon_Handler:   b DebugMon_Handler
    .thumb_func
PendSV_Handler:     b PendSV_Handler
    .thumb_func
SysTick_Handler:    b SysTick_Handler
