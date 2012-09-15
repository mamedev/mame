#pragma once

#ifndef __PDP1_H__
#define __PDP1_H__



/* register ids for pdp1_get_reg/pdp1_set_reg */
enum
{
	PDP1_PC=1, PDP1_IR, PDP1_MB, PDP1_MA, PDP1_AC, PDP1_IO,
	PDP1_PF, PDP1_PF1, PDP1_PF2, PDP1_PF3, PDP1_PF4, PDP1_PF5, PDP1_PF6,
	PDP1_TA, PDP1_TW,
	PDP1_SS, PDP1_SS1, PDP1_SS2, PDP1_SS3, PDP1_SS4, PDP1_SS5, PDP1_SS6,
	PDP1_SNGL_STEP, PDP1_SNGL_INST, PDP1_EXTEND_SW,
	PDP1_RUN, PDP1_CYC, PDP1_DEFER, PDP1_BRK_CTR, PDP1_OV, PDP1_RIM, PDP1_SBM, PDP1_EXD,
	PDP1_IOC, PDP1_IOH, PDP1_IOS,
	PDP1_START_CLEAR,	/* hack, do not use directly, use pdp1_pulse_start_clear instead */
	PDP1_IO_COMPLETE	/* hack, do not use directly, use pdp1_pulse_iot_done instead */
};

#define pdp1_pulse_start_clear(cpudevice)	(cpudevice)->state().set_state_int(PDP1_START_CLEAR, (UINT64)0)
#define pdp1_pulse_iot_done(cpudevice)		(cpudevice)->state().set_state_int(PDP1_IO_COMPLETE, (UINT64)0)

typedef void (*pdp1_extern_iot_func)(device_t *device, int op2, int nac, int mb, int *io, int ac);
typedef void (*pdp1_read_binary_word_func)(device_t *device);
typedef void (*pdp1_io_sc_func)(device_t *device);


struct pdp1_reset_param_t
{
	/* callbacks for iot instructions (required for any I/O) */
	pdp1_extern_iot_func extern_iot[64];
	/* read a word from the perforated tape reader (required for read-in mode) */
	pdp1_read_binary_word_func read_binary_word;
	/* callback called when sc is pulsed: IO devices should reset */
	pdp1_io_sc_func io_sc_callback;

	/* 0: no extend support, 1: extend with 15-bit address, 2: extend with 16-bit address */
	int extend_support;
	/* 1 to use hardware multiply/divide (MUL, DIV) instead of MUS, DIS */
	int hw_mul_div;
	/* 0: standard sequence break system 1: type 20 sequence break system */
	int type_20_sbs;
};

#define IOT_NO_COMPLETION_PULSE -1

/* PUBLIC FUNCTIONS */
DECLARE_LEGACY_CPU_DEVICE(PDP1, pdp1);

#define AND 001
#define IOR 002
#define XOR 003
#define XCT 004
#define CALJDA 007
#define LAC 010
#define LIO 011
#define DAC 012
#define DAP 013
#define DIP 014
#define DIO 015
#define DZM 016
#define ADD 020
#define SUB 021
#define IDX 022
#define ISP 023
#define SAD 024
#define SAS 025
#define MUS_MUL 026
#define DIS_DIV 027
#define JMP 030
#define JSP 031
#define SKP 032
#define SFT 033
#define LAW 034
#define IOT 035
#define OPR 037

CPU_DISASSEMBLE( pdp1 );

#endif /* __PDP1_H__ */
