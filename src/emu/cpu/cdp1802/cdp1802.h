#pragma once

#ifndef __CDP1802_H__
#define __CDP1802_H__

#include "cpuintrf.h"

enum
{
	CDP1802_INPUT_LINE_INT,
	CDP1802_INPUT_LINE_DMAIN,
	CDP1802_INPUT_LINE_DMAOUT
};

enum
{
	EF1 = 0x01,
	EF2 = 0x02,
	EF3 = 0x04,
	EF4 = 0x08
};

enum _cdp1802_control_mode {
	CDP1802_MODE_LOAD,
	CDP1802_MODE_RESET,
	CDP1802_MODE_PAUSE,
	CDP1802_MODE_RUN
};
typedef enum _cdp1802_control_mode cdp1802_control_mode;

enum _cdp1802_state {
	CDP1802_STATE_CODE_S0_FETCH,
	CDP1802_STATE_CODE_S1_EXECUTE,
	CDP1802_STATE_CODE_S2_DMA,
	CDP1802_STATE_CODE_S3_INTERRUPT
};
typedef enum _cdp1802_state cdp1802_state;

// CDP1802 Registers

enum
{
	CDP1802_PC = 1,
	CDP1802_P,		// Designates which register is Program Counter
	CDP1802_X,		// Designates which register is Data Pointer
	CDP1802_D,		// Data Register (Accumulator)
	CDP1802_B,		// Auxiliary Holding Register
	CDP1802_T,		// Holds old X, P after Interrupt (X is high nibble)

	CDP1802_R0,		// 1 of 16 Scratchpad Registers
	CDP1802_R1,
	CDP1802_R2,
	CDP1802_R3,
	CDP1802_R4,
	CDP1802_R5,
	CDP1802_R6,
	CDP1802_R7,
	CDP1802_R8,
	CDP1802_R9,
	CDP1802_Ra,
	CDP1802_Rb,
	CDP1802_Rc,
	CDP1802_Rd,
	CDP1802_Re,
	CDP1802_Rf,

	CDP1802_DF,		// Data Flag (ALU Carry)
	CDP1802_IE,		// Interrupt Enable
	CDP1802_Q,		// Output Flip-Flop
	CDP1802_N,		// Holds Low-Order Instruction Digit
	CDP1802_I,		// Holds High-Order Instruction Digit
};

void cdp1802_get_info(UINT32 state, cpuinfo *info);

typedef cdp1802_control_mode (*cdp1802_mode_read_func)(running_machine *machine);
#define CDP1802_MODE_READ(name) cdp1802_control_mode name(running_machine *machine)

typedef UINT8 (*cdp1802_ef_read_func)(running_machine *machine);
#define CDP1802_EF_READ(name) UINT8 name(running_machine *machine)

typedef void (*cdp1802_sc_write_func)(running_machine *machine, cdp1802_state state);
#define CDP1802_SC_WRITE(name) void name(running_machine *machine, cdp1802_state state)

typedef void (*cdp1802_q_write_func)(running_machine *machine, int level);
#define CDP1802_Q_WRITE(name) void name(running_machine *machine, int level)

typedef UINT8 (*cdp1802_dma_read_func)(running_machine *machine, UINT16 ma);
#define CDP1802_DMA_READ(name) UINT8 name(running_machine *machine, UINT16 ma)

typedef void (*cdp1802_dma_write_func)(running_machine *machine, UINT16 ma, UINT8 data);
#define CDP1802_DMA_WRITE(name) void name(running_machine *machine, UINT16 ma, UINT8 data)

/* interface */
typedef struct _cdp1802_interface cdp1802_interface;
struct _cdp1802_interface
{
	/* if specified, this gets called for every change of the mode pins (pins 2 and 3) */
	cdp1802_mode_read_func	mode_r;

	/* if specified, this gets called for every change read of the external flags (pins 21 thru 24) */
	cdp1802_ef_read_func	ef_r;

	/* if specified, this gets called for every change of the processor state (pins 5 and 6) */
	cdp1802_sc_write_func	sc_w;

	/* if specified, this gets called for every change of the Q pin (pin 4) */
	cdp1802_q_write_func	q_w;

	/* if specified, this gets called for every DMA read */
	cdp1802_dma_read_func	dma_r;

	/* if specified, this gets called for every DMA write */
	cdp1802_dma_write_func	dma_w;
};
#define CDP1802_INTERFACE(name) const cdp1802_interface (name) =

offs_t cdp1802_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif /* __CDP1802_H__ */
