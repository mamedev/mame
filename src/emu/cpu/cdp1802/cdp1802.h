#pragma once

#ifndef __CDP1802_H__
#define __CDP1802_H__

/*

    RCA COSMAC Series Microprocessors

    Type            Internal ROM    Internal RAM    Timer
    -----------------------------------------------------
    CDP1802         none            none            no
    CDP1803         ?               ?               ?
    CDP1804         2 KB            64 bytes        yes
    CDP1805         none            64 bytes        yes
    CDP1806         none            none            yes

*/

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

enum _cdp1802_control_mode
{
	CDP1802_MODE_LOAD,
	CDP1802_MODE_RESET,
	CDP1802_MODE_PAUSE,
	CDP1802_MODE_RUN
};
typedef enum _cdp1802_control_mode cdp1802_control_mode;

enum _cdp1802_state_code
{
	CDP1802_STATE_CODE_S0_FETCH = 0,
	CDP1802_STATE_CODE_S1_EXECUTE,
	CDP1802_STATE_CODE_S2_DMA,
	CDP1802_STATE_CODE_S3_INTERRUPT
};
typedef enum _cdp1802_state_code cdp1802_state_code;

enum
{
	CDP1802_P, CDP1802_X, CDP1802_D, CDP1802_B, CDP1802_T,
	CDP1802_R0, CDP1802_R1, CDP1802_R2, CDP1802_R3, CDP1802_R4, CDP1802_R5, CDP1802_R6, CDP1802_R7,
	CDP1802_R8, CDP1802_R9, CDP1802_Ra, CDP1802_Rb, CDP1802_Rc, CDP1802_Rd, CDP1802_Re, CDP1802_Rf,
	CDP1802_DF, CDP1802_IE, CDP1802_Q, CDP1802_N, CDP1802_I, CDP1802_SC,
	CDP1802_GENPC = STATE_GENPC
};

typedef cdp1802_control_mode (*cdp1802_mode_read_func)(running_device *device);
#define CDP1802_MODE_READ(name) cdp1802_control_mode name(running_device *device)

typedef UINT8 (*cdp1802_ef_read_func)(running_device *device);
#define CDP1802_EF_READ(name) UINT8 name(running_device *device)

typedef void (*cdp1802_sc_write_func)(running_device *device, cdp1802_state_code state, int sc0, int sc1);
#define CDP1802_SC_WRITE(name) void name(running_device *device, cdp1802_state_code state, int sc0, int sc1)

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
	devcb_write_line		out_q_func;

	/* if specified, this gets called for every DMA read */
	devcb_read8				in_dma_func;

	/* if specified, this gets called for every DMA write */
	devcb_write8			out_dma_func;
};
#define CDP1802_INTERFACE(name) const cdp1802_interface (name) =

DECLARE_LEGACY_CPU_DEVICE(CDP1802, cdp1802);

extern CPU_DISASSEMBLE( cdp1802 );

#endif /* __CDP1802_H__ */
