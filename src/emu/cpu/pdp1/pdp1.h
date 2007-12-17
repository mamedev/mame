#ifndef _PDP1_H
#define _PDP1_H

#include "cpuintrf.h"


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

#define pdp1_pulse_start_clear()	cpunum_set_reg(0, PDP1_START_CLEAR, 0)
#define pdp1_pulse_iot_done()		cpunum_set_reg(0, PDP1_IO_COMPLETE, 0)

typedef struct pdp1_reset_param_t
{
	/* callbacks for iot instructions (required for any I/O) */
	void (*extern_iot[64])(int op2, int nac, int mb, int *io, int ac);
	/* read a word from the perforated tape reader (required for read-in mode) */
	void (*read_binary_word)(void);
	/* callback called when sc is pulsed: IO devices should reset */
	void (*io_sc_callback)(void);

	/* 0: no extend support, 1: extend with 15-bit address, 2: extend with 16-bit address */
	int extend_support;
	/* 1 to use hardware multiply/divide (MUL, DIV) instead of MUS, DIS */
	int hw_mul_div;
	/* 0: standard sequence break system 1: type 20 sequence break system */
	int type_20_sbs;
} pdp1_reset_param_t;

#define IOT_NO_COMPLETION_PULSE -1

/* PUBLIC FUNCTIONS */
void pdp1_get_info(UINT32 state, cpuinfo *info);

#define READ_PDP_18BIT(A) ((signed)program_read_dword_32be((A)<<2))
#define WRITE_PDP_18BIT(A,V) (program_write_dword_32be((A)<<2,(V)))

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

#ifdef MAME_DEBUG
unsigned pdp1_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif /* MAME_DEBUG */

#endif /* _PDP1_H */
