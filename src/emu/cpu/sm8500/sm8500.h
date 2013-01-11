#pragma once

#ifndef __SM8500_H__
#define __SM8500_H__


struct SM8500_CONFIG {
	void (*handle_dma)(device_t *device, int cycles);
	void (*handle_timers)(device_t *device, int cycles);
};

/* interrupts */
#define ILL_INT         0
#define DMA_INT         1
#define TIM0_INT        2
#define EXT_INT         3
#define UART_INT        4
#define LCDC_INT        5
#define TIM1_INT        6
#define CK_INT          7
#define PIO_INT         8
#define WDT_INT         9
#define NMI_INT         10

enum
{
	/* "main" 16 bit register */
		SM8500_PC=1, SM8500_SP, SM8500_PS, SM8500_SYS16, SM8500_RR0, SM8500_RR2, SM8500_RR4, SM8500_RR6, SM8500_RR8, SM8500_RR10,
	SM8500_RR12, SM8500_RR14,
	/* additional internal 8 bit registers */
	SM8500_IE0, SM8500_IE1, SM8500_IR0, SM8500_IR1, SM8500_P0, SM8500_P1, SM8500_P2, SM8500_P3, SM8500_SYS, SM8500_CKC,
	SM8500_SPH, SM8500_SPL, SM8500_PS0, SM8500_PS1, SM8500_P0C, SM8500_P1C, SM8500_P2C, SM8500_P3C,
};

DECLARE_LEGACY_CPU_DEVICE(SM8500, sm8500);

extern CPU_DISASSEMBLE( sm8500 );

#endif /* __SM8500_H__ */
