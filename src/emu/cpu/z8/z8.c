/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - strobed I/O
    - interrupts
    - expose register file to disassembler
    - decimal adjust instruction
    - timer Tin/Tout modes
    - serial
    - instruction pipeline

*/

#include "emu.h"
#include "debugger.h"
#include "z8.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	Z8_REGISTER_P0 = 0,
	Z8_REGISTER_P1,
	Z8_REGISTER_P2,
	Z8_REGISTER_P3,
	Z8_REGISTER_SIO = 0xf0,
	Z8_REGISTER_TMR,
	Z8_REGISTER_T1,
	Z8_REGISTER_PRE1,
	Z8_REGISTER_T0,
	Z8_REGISTER_PRE0,
	Z8_REGISTER_P2M,
	Z8_REGISTER_P3M,
	Z8_REGISTER_P01M,
	Z8_REGISTER_IPR,
	Z8_REGISTER_IRQ,
	Z8_REGISTER_IMR,
	Z8_REGISTER_FLAGS,
	Z8_REGISTER_RP,
	Z8_REGISTER_SPH,
	Z8_REGISTER_SPL
};

#define Z8_P3_DAV0					0x04	/* not supported */
#define Z8_P3_DAV1					0x08	/* not supported */
#define Z8_P3_DAV2					0x02	/* not supported */
#define Z8_P3_RDY0					0x20	/* not supported */
#define Z8_P3_RDY1					0x10	/* not supported */
#define Z8_P3_RDY2					0x40	/* not supported */
#define Z8_P3_IRQ0					0x04	/* not supported */
#define Z8_P3_IRQ1					0x08	/* not supported */
#define Z8_P3_IRQ2					0x02	/* not supported */
#define Z8_P3_IRQ3					0x01	/* not supported */
#define Z8_P3_DI					0x01	/* not supported */
#define Z8_P3_DO					0x80	/* not supported */
#define Z8_P3_TIN					0x02	/* not supported */
#define Z8_P3_TOUT					0x40	/* not supported */
#define Z8_P3_DM					0x10	/* not supported */

#define Z8_PRE0_COUNT_MODULO_N		0x01

#define Z8_PRE1_COUNT_MODULO_N		0x01
#define Z8_PRE1_INTERNAL_CLOCK		0x02

#define Z8_TMR_LOAD_T0				0x01
#define Z8_TMR_ENABLE_T0			0x02
#define Z8_TMR_LOAD_T1				0x04
#define Z8_TMR_ENABLE_T1			0x08
#define Z8_TMR_TIN_MASK				0x30	/* not supported */
#define Z8_TMR_TIN_EXTERNAL_CLK		0x00	/* not supported */
#define Z8_TMR_TIN_GATE				0x10	/* not supported */
#define Z8_TMR_TIN_TRIGGER			0x20	/* not supported */
#define Z8_TMR_TIN_RETRIGGER		0x30	/* not supported */
#define Z8_TMR_TOUT_MASK			0xc0	/* not supported */
#define Z8_TMR_TOUT_OFF				0x00	/* not supported */
#define Z8_TMR_TOUT_T0				0x40	/* not supported */
#define Z8_TMR_TOUT_T1				0x80	/* not supported */
#define Z8_TMR_TOUT_INTERNAL_CLK	0xc0	/* not supported */

#define Z8_P01M_P0L_MODE_MASK		0x03
#define Z8_P01M_P0L_MODE_OUTPUT		0x00
#define Z8_P01M_P0L_MODE_INPUT		0x01
#define Z8_P01M_P0L_MODE_A8_A11		0x02	/* not supported */
#define Z8_P01M_INTERNAL_STACK		0x04
#define Z8_P01M_P1_MODE_MASK		0x18
#define Z8_P01M_P1_MODE_OUTPUT		0x00
#define Z8_P01M_P1_MODE_INPUT		0x08
#define Z8_P01M_P1_MODE_AD0_AD7		0x10	/* not supported */
#define Z8_P01M_P1_MODE_HI_Z		0x18	/* not supported */
#define Z8_P01M_EXTENDED_TIMING		0x20	/* not supported */
#define Z8_P01M_P0H_MODE_MASK		0xc0
#define Z8_P01M_P0H_MODE_OUTPUT		0x00
#define Z8_P01M_P0H_MODE_INPUT		0x40
#define Z8_P01M_P0H_MODE_A12_A15	0x80	/* not supported */

#define Z8_P3M_P2_ACTIVE_PULLUPS	0x01	/* not supported */
#define Z8_P3M_P0_STROBED			0x04	/* not supported */
#define Z8_P3M_P33_P34_MASK			0x18
#define Z8_P3M_P33_P34_INPUT_OUTPUT	0x00
#define Z8_P3M_P33_P34_INPUT_DM		0x08	/* not supported */
#define Z8_P3M_P33_P34_INPUT_DM_2	0x10	/* not supported */
#define Z8_P3M_P33_P34_DAV1_RDY1	0x18	/* not supported */
#define Z8_P3M_P2_STROBED			0x20	/* not supported */
#define Z8_P3M_P3_SERIAL			0x40	/* not supported */
#define Z8_P3M_PARITY				0x80	/* not supported */

#define Z8_IMR_ENABLE				0x80	/* not supported */
#define Z8_IMR_RAM_PROTECT			0x40	/* not supported */
#define Z8_IMR_ENABLE_IRQ5			0x20	/* not supported */
#define Z8_IMR_ENABLE_IRQ4			0x10	/* not supported */
#define Z8_IMR_ENABLE_IRQ3			0x08	/* not supported */
#define Z8_IMR_ENABLE_IRQ2			0x04	/* not supported */
#define Z8_IMR_ENABLE_IRQ1			0x02	/* not supported */
#define Z8_IMR_ENABLE_IRQ0			0x01	/* not supported */

#define Z8_FLAGS_F1					0x01
#define Z8_FLAGS_F2					0x02
#define Z8_FLAGS_H					0x04
#define Z8_FLAGS_D					0x08
#define Z8_FLAGS_V					0x10
#define Z8_FLAGS_S					0x20
#define Z8_FLAGS_Z					0x40
#define Z8_FLAGS_C					0x80

enum
{
	CC_F = 0, CC_LT, CC_LE, CC_ULE, CC_OV, CC_MI, CC_Z, CC_C,
	CC_T, CC_GE, CC_GT, CC_UGT, CC_NOV, CC_PL, CC_NZ, CC_NC
};

/***************************************************************************
    MACROS
***************************************************************************/

#define P01M		cpustate->r[Z8_REGISTER_P01M]
#define P2M			cpustate->r[Z8_REGISTER_P2M]
#define P3M			cpustate->r[Z8_REGISTER_P3M]
#define T0			cpustate->r[Z8_REGISTER_T0]
#define T1			cpustate->r[Z8_REGISTER_T1]
#define PRE0		cpustate->r[Z8_REGISTER_PRE0]
#define PRE1		cpustate->r[Z8_REGISTER_PRE1]

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct z8_state
{
    address_space *program;
    direct_read_data *direct;
    address_space *data;
    address_space *io;

	/* registers */
	UINT16 pc;				/* program counter */
	UINT8 r[256];			/* register file */
	UINT8 input[4];			/* port input latches */
	UINT8 output[4];		/* port output latches */
	UINT8 t0;				/* timer 0 current count */
	UINT8 t1;				/* timer 1 current count */

	/* fake registers */
	UINT16 fake_sp;			/* fake stack pointer */
	UINT8 fake_r[16];		/* fake working registers */

	/* interrupts */
	int irq[6];				/* interrupts */

	/* execution logic */
	int clock;				/* clock */
	int icount;				/* instruction counter */

	/* timers */
	emu_timer *t0_timer;
	emu_timer *t1_timer;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z8_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == Z8601) ||
		(device->type() == UB8830D) ||
		(device->type() == Z8611));
	return (z8_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE UINT8 fetch(z8_state *cpustate)
{
	UINT8 data = cpustate->direct->read_decrypted_byte(cpustate->pc);

	cpustate->pc++;

	return data;
}

INLINE UINT8 register_read(z8_state *cpustate, UINT8 offset)
{
	UINT8 data = 0xff;
	UINT8 mask = 0;

	switch (offset)
	{
	case Z8_REGISTER_P0:
		switch (P01M & Z8_P01M_P0L_MODE_MASK)
		{
		case Z8_P01M_P0L_MODE_OUTPUT:	data = cpustate->output[offset] & 0x0f;		break;
		case Z8_P01M_P0L_MODE_INPUT:	mask = 0x0f;								break;
		default: /* A8...A11 */			data = 0x0f;								break;
		}

		switch (P01M & Z8_P01M_P0H_MODE_MASK)
		{
		case Z8_P01M_P0H_MODE_OUTPUT:	data |= cpustate->output[offset] & 0xf0;	break;
		case Z8_P01M_P0H_MODE_INPUT:	mask |= 0xf0;								break;
		default: /* A12...A15 */		data |= 0xf0;								break;
		}

		if (!(P3M & Z8_P3M_P0_STROBED))
		{
			if (mask) cpustate->input[offset] = cpustate->io->read_byte(offset);
		}

		data |= cpustate->input[offset] & mask;
		break;

	case Z8_REGISTER_P1:
		switch (P01M & Z8_P01M_P1_MODE_MASK)
		{
		case Z8_P01M_P1_MODE_OUTPUT:	data = cpustate->output[offset];			break;
		case Z8_P01M_P1_MODE_INPUT:		mask = 0xff;								break;
		default: /* AD0..AD7 */			data = 0xff;								break;
		}

		if ((P3M & Z8_P3M_P33_P34_MASK) != Z8_P3M_P33_P34_DAV1_RDY1)
		{
			if (mask) cpustate->input[offset] = cpustate->io->read_byte(offset);
		}

		data |= cpustate->input[offset] & mask;
		break;

	case Z8_REGISTER_P2:
		mask = cpustate->r[Z8_REGISTER_P2M];

		if (!(P3M & Z8_P3M_P2_STROBED))
		{
			if (mask) cpustate->input[offset] = cpustate->io->read_byte(offset);
		}

		data = (cpustate->input[offset] & mask) | (cpustate->output[offset] & ~mask);
		break;

	case Z8_REGISTER_P3:
		// TODO: special port 3 modes
		if (!(P3M & 0x7c))
		{
			mask = 0x0f;
		}

		if (mask) cpustate->input[offset] = cpustate->io->read_byte(offset);

		data = (cpustate->input[offset] & mask) | (cpustate->output[offset] & ~mask);
		break;

	case Z8_REGISTER_T0:
		data = cpustate->t0;
		break;

	case Z8_REGISTER_T1:
		data = cpustate->t1;
		break;

	case Z8_REGISTER_PRE1:
	case Z8_REGISTER_PRE0:
	case Z8_REGISTER_P2M:
	case Z8_REGISTER_P3M:
	case Z8_REGISTER_P01M:
	case Z8_REGISTER_IPR:
		/* write only */
		break;

	default:
		data = cpustate->r[offset];
		break;
	}

	return data;
}

INLINE UINT16 register_pair_read(z8_state *cpustate, UINT8 offset)
{
	return (register_read(cpustate, offset) << 8) | register_read(cpustate, offset + 1);
}

INLINE void register_write(z8_state *cpustate, UINT8 offset, UINT8 data)
{
	UINT8 mask = 0;

	switch (offset)
	{
	case Z8_REGISTER_P0:
		cpustate->output[offset] = data;
		if ((P01M & Z8_P01M_P0L_MODE_MASK) == Z8_P01M_P0L_MODE_OUTPUT) mask |= 0x0f;
		if ((P01M & Z8_P01M_P0H_MODE_MASK) == Z8_P01M_P0H_MODE_OUTPUT) mask |= 0xf0;
		if (mask) cpustate->io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_P1:
		cpustate->output[offset] = data;
		if ((P01M & Z8_P01M_P1_MODE_MASK) == Z8_P01M_P1_MODE_OUTPUT) mask = 0xff;
		if (mask) cpustate->io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_P2:
		cpustate->output[offset] = data;
		mask = cpustate->r[Z8_REGISTER_P2M] ^ 0xff;
		if (mask) cpustate->io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_P3:
		cpustate->output[offset] = data;

		// TODO: special port 3 modes
		if (!(P3M & 0x7c))
		{
			mask = 0xf0;
		}

		if (mask) cpustate->io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_SIO:
		break;

	case Z8_REGISTER_TMR:
		if (data & Z8_TMR_LOAD_T0)
		{
			cpustate->t0 = T0;
			cpustate->t0_timer->adjust(attotime::zero, 0, attotime::from_hz(cpustate->clock / 2 / 4 / ((PRE0 >> 2) + 1)));
		}

		cpustate->t0_timer->enable(data & Z8_TMR_ENABLE_T0);

		if (data & Z8_TMR_LOAD_T1)
		{
			cpustate->t1 = T1;
			cpustate->t1_timer->adjust(attotime::zero, 0, attotime::from_hz(cpustate->clock / 2 / 4 / ((PRE1 >> 2) + 1)));
		}

		cpustate->t1_timer->enable(data & Z8_TMR_ENABLE_T1);
		break;

	case Z8_REGISTER_P2M:
		break;
	case Z8_REGISTER_P3M:
		break;
	case Z8_REGISTER_P01M:
		break;
	case Z8_REGISTER_IPR:
		break;
	case Z8_REGISTER_IRQ:
		break;
	case Z8_REGISTER_IMR:
		break;
	case Z8_REGISTER_FLAGS:
		break;
	case Z8_REGISTER_RP:
		break;
	case Z8_REGISTER_SPH:
		break;
	case Z8_REGISTER_SPL:
		break;
	default:
		// TODO ignore missing registers
		break;
	}

	cpustate->r[offset] = data;
}

INLINE void register_pair_write(z8_state *cpustate, UINT8 offset, UINT16 data)
{
	register_write(cpustate, offset, data >> 8);
	register_write(cpustate, offset + 1, data & 0xff);
}

INLINE UINT8 get_working_register(z8_state *cpustate, int offset)
{
	return (cpustate->r[Z8_REGISTER_RP] & 0xf0) | (offset & 0x0f);
}

INLINE UINT8 get_register(z8_state *cpustate, UINT8 offset)
{
	if ((offset & 0xf0) == 0xe0)
		return get_working_register(cpustate, offset & 0x0f);
	else
		return offset;
}

INLINE UINT8 get_intermediate_register(z8_state *cpustate, int offset)
{
	return register_read(cpustate, get_register(cpustate, offset));
}

INLINE void stack_push_byte(z8_state *cpustate, UINT8 src)
{
	if (register_read(cpustate, Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP - 1 */
		UINT8 sp = register_read(cpustate, Z8_REGISTER_SPL) - 1;
		register_write(cpustate, Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		register_write(cpustate, sp, src);
	}
	else
	{
		/* SP <- SP - 1 */
		UINT16 sp = register_pair_read(cpustate, Z8_REGISTER_SPH) - 1;
		register_pair_write(cpustate, Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		cpustate->data->write_byte(sp, src);
	}
}

INLINE void stack_push_word(z8_state *cpustate, UINT16 src)
{
	if (register_read(cpustate, Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP - 2 */
		UINT8 sp = register_read(cpustate, Z8_REGISTER_SPL) - 2;
		register_write(cpustate, Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		register_pair_write(cpustate, sp, src);
	}
	else
	{
		/* SP <- SP - 2 */
		UINT16 sp = register_pair_read(cpustate, Z8_REGISTER_SPH) - 2;
		register_pair_write(cpustate, Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		cpustate->data->write_word(sp, src);
	}
}

INLINE UINT8 stack_pop_byte(z8_state *cpustate)
{
	if (register_read(cpustate, Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP + 1 */
		UINT8 sp = register_read(cpustate, Z8_REGISTER_SPL) + 1;
		register_write(cpustate, Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		return register_read(cpustate, sp);
	}
	else
	{
		/* SP <- SP + 1 */
		UINT16 sp = register_pair_read(cpustate, Z8_REGISTER_SPH) + 1;
		register_pair_write(cpustate, Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		return cpustate->data->read_byte(sp);
	}
}

INLINE UINT16 stack_pop_word(z8_state *cpustate)
{
	if (register_read(cpustate, Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP + 2 */
		UINT8 sp = register_read(cpustate, Z8_REGISTER_SPL) + 2;
		register_write(cpustate, Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		return register_read(cpustate, sp);
	}
	else
	{
		/* SP <- SP + 2 */
		UINT16 sp = register_pair_read(cpustate, Z8_REGISTER_SPH) + 2;
		register_pair_write(cpustate, Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		return cpustate->data->read_word(sp);
	}
}

INLINE void set_flag(z8_state *cpustate, UINT8 flag, int state)
{
	if (state)
		cpustate->r[Z8_REGISTER_FLAGS] |= flag;
	else
		cpustate->r[Z8_REGISTER_FLAGS] &= ~flag;
}

#define set_flag_h(state)	set_flag(cpustate, Z8_FLAGS_H, state);
#define set_flag_d(state)	set_flag(cpustate, Z8_FLAGS_D, state);
#define set_flag_v(state)	set_flag(cpustate, Z8_FLAGS_V, state);
#define set_flag_s(state)	set_flag(cpustate, Z8_FLAGS_S, state);
#define set_flag_z(state)	set_flag(cpustate, Z8_FLAGS_Z, state);
#define set_flag_c(state)	set_flag(cpustate, Z8_FLAGS_C, state);

/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define INSTRUCTION(mnemonic) INLINE void (mnemonic)(z8_state *cpustate, UINT8 opcode, int *cycles)

INSTRUCTION( illegal )
{
	logerror("Z8: PC = %04x, Illegal opcode = %02x\n", cpustate->pc - 1, opcode);
}

#include "z8ops.c"

/***************************************************************************
    OPCODE TABLES
***************************************************************************/

typedef void (*z8_opcode_func) (z8_state *cpustate, UINT8 opcode, int *cycles);

struct z8_opcode_map
{
	z8_opcode_func	function;
	int				execution_cycles;
	int				pipeline_cycles;
};

static const z8_opcode_map Z8601_OPCODE_MAP[] =
{
	{ dec_R1, 6, 5 },	{ dec_IR1, 6, 5 },	{ add_r1_r2, 10, 5 },	{ add_r1_Ir2, 10, 5 },	{ add_R2_R1, 10, 5 },	{ add_IR2_R1, 10, 5 },	{ add_R1_IM, 10, 5 },	{ add_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ rlc_R1, 6, 5 },	{ rlc_IR1, 6, 5 },	{ adc_r1_r2, 6, 5 },	{ adc_r1_Ir2, 6, 5 },	{ adc_R2_R1, 10, 5 },	{ adc_IR2_R1, 10, 5 },	{ adc_R1_IM, 10, 5 },	{ adc_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ inc_R1, 6, 5 },	{ inc_IR1, 6, 5 },	{ sub_r1_r2, 6, 5 },	{ sub_r1_Ir2, 6, 5 },	{ sub_R2_R1, 10, 5 },	{ sub_IR2_R1, 10, 5 },	{ sub_R1_IM, 10, 5 },	{ sub_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ jp_IRR1, 8, 0 },	{ srp_IM, 6, 1 },	{ sbc_r1_r2, 6, 5 },	{ sbc_r1_Ir2, 6, 5 },	{ sbc_R2_R1, 10, 5 },	{ sbc_IR2_R1, 10, 5 },	{ sbc_R1_IM, 10, 5 },	{ sbc_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ da_R1, 8, 5 },	{ da_IR1, 8, 5 },	{ or_r1_r2, 6, 5 },		{ or_r1_Ir2, 6, 5 },	{ or_R2_R1, 10, 5 },	{ or_IR2_R1, 10, 5 },	{ or_R1_IM, 10, 5 },	{ or_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ pop_R1, 10, 5 },	{ pop_IR1, 10, 5 },	{ and_r1_r2, 6, 5 },	{ and_r1_Ir2, 6, 5 },	{ and_R2_R1, 10, 5 },	{ and_IR2_R1, 10, 5 },	{ and_R1_IM, 10, 5 },	{ and_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ com_R1, 6, 5 },	{ com_IR1, 6, 5 },	{ tcm_r1_r2, 6, 5 },	{ tcm_r1_Ir2, 6, 5 },	{ tcm_R2_R1, 10, 5 },	{ tcm_IR2_R1, 10, 5 },	{ tcm_R1_IM, 10, 5 },	{ tcm_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ push_R2, 10, 1 },	{ push_IR2, 12, 1 },{ tm_r1_r2, 6, 5 },		{ tm_r1_Ir2, 6, 5 },	{ tm_R2_R1, 10, 5 },	{ tm_IR2_R1, 10, 5 },	{ tm_R1_IM, 10, 5 },	{ tm_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ illegal, 0, 0 },

	{ decw_RR1, 10, 5 },{ decw_IR1, 10, 5 },{ lde_r1_Irr2, 12, 0 },	{ ldei_Ir1_Irr2, 18, 0 },{ illegal, 0, 0 },		{ illegal, 0, 0 },		{ illegal, 0, 0 },		{ illegal, 0, 0 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ di, 6, 1 },

	{ rl_R1, 6, 5 },	{ rl_IR1, 6, 5 },	{ lde_r2_Irr1, 12, 0 },	{ ldei_Ir2_Irr1, 18, 0 },{ illegal, 0, 0 },		{ illegal, 0, 0 },		{ illegal, 0, 0 },		{ illegal, 0, 0 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ ei, 6, 1 },

	{ incw_RR1, 10, 5 },{ incw_IR1, 10, 5 },{ cp_r1_r2, 6, 5 },		{ cp_r1_Ir2, 6, 5 },	{ cp_R2_R1, 10, 5 },	{ cp_IR2_R1, 10, 5 },	{ cp_R1_IM, 10, 5 },	{ cp_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ ret, 14, 0 },

	{ clr_R1, 6, 5 },	{ clr_IR1, 6, 5 },	{ xor_r1_r2, 6, 5 },	{ xor_r1_Ir2, 6, 5 },	{ xor_R2_R1, 10, 5 },	{ xor_IR2_R1, 10, 5 },	{ xor_R1_IM, 10, 5 },	{ xor_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ iret, 16, 0 },

	{ rrc_R1, 6, 5 },	{ rrc_IR1, 6, 5 },	{ ldc_r1_Irr2, 12, 0 },	{ ldci_Ir1_Irr2, 18, 0 },{ illegal, 0, 0 },		{ illegal, 0, 0 },		{ illegal, 0, 0 },		{ ld_r1_x_R2, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ rcf, 6, 5 },

	{ sra_R1, 6, 5 },	{ sra_IR1, 6, 5 },	{ ldc_r2_Irr1, 12, 0 },	{ ldci_Ir2_Irr1, 18, 0 },{ call_IRR1, 20, 0 },	{ illegal, 0, 0 },		{ call_DA, 20, 0 },		{ ld_r2_x_R1, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ scf, 6, 5 },

	{ rr_R1, 6, 5 },	{ rr_IR1, 6, 5 },	{ illegal, 0, 0 },		{ ld_r1_Ir2, 6, 5 },	{ ld_R2_R1, 10, 5 },	{ ld_IR2_R1, 10, 5 },	{ ld_R1_IM, 10, 5 },	{ ld_IR1_IM, 10, 5 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ ccf, 6, 5 },

	{ swap_R1, 8, 5 },	{ swap_IR1, 8, 5 },	{ illegal, 0, 0 },		{ ld_Ir1_r2, 6, 5 },	{ illegal, 0, 0 },		{ ld_R2_IR1, 10, 5 },	{ illegal, 0, 0 },		{ illegal, 0, 0 },
	{ ld_r1_R2, 6, 5 }, { ld_r2_R1, 6, 5 }, { djnz_r1_RA, 10, 5 },	{ jr_cc_RA, 10, 0 },	{ ld_r1_IM, 6, 5 },		{ jp_cc_DA, 10, 0 },	{ inc_r1, 6, 5 },		{ nop, 6, 0 },
};

/***************************************************************************
    TIMER CALLBACKS
***************************************************************************/

static TIMER_CALLBACK( t0_tick )
{
	z8_state *cpustate = (z8_state *)ptr;

	cpustate->t0--;

	if (cpustate->t0 == 0)
	{
		cpustate->t0 = T0;
		cpustate->t0_timer->adjust(attotime::zero, 0, attotime::from_hz(cpustate->clock / 2 / 4 / ((PRE0 >> 2) + 1)));
		cpustate->t0_timer->enable(PRE0 & Z8_PRE0_COUNT_MODULO_N);
		cpustate->irq[4] = ASSERT_LINE;
	}
}

static TIMER_CALLBACK( t1_tick )
{
	z8_state *cpustate = (z8_state *)ptr;

	cpustate->t1--;

	if (cpustate->t1 == 0)
	{
		cpustate->t1 = T1;
		cpustate->t1_timer->adjust(attotime::zero, 0, attotime::from_hz(cpustate->clock / 2 / 4 / ((PRE1 >> 2) + 1)));
		cpustate->t1_timer->enable(PRE1 & Z8_PRE0_COUNT_MODULO_N);
		cpustate->irq[5] = ASSERT_LINE;
	}
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

static CPU_INIT( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	/* set up the state table */
	{
		device_state_interface *state;
		device->interface(state);
		state->state_add(Z8_PC,         "PC",        cpustate->pc);
		state->state_add(STATE_GENPC,   "GENPC",     cpustate->pc).noshow();
		state->state_add(Z8_SP,         "SP",        cpustate->fake_sp).callimport().callexport();
		state->state_add(STATE_GENSP,   "GENSP",     cpustate->fake_sp).callimport().callexport().noshow();
		state->state_add(Z8_RP,         "RP",        cpustate->r[Z8_REGISTER_RP]);
		state->state_add(Z8_T0,         "T0",        cpustate->t0);
		state->state_add(Z8_T1,         "T1",        cpustate->t1);
		state->state_add(STATE_GENFLAGS, "GENFLAGS", cpustate->r[Z8_REGISTER_FLAGS]).noshow().formatstr("%6s");

		astring tempstr;
		for (int regnum = 0; regnum < 16; regnum++)
			state->state_add(Z8_R0 + regnum, tempstr.format("R%d", regnum), cpustate->fake_r[regnum]).callimport().callexport();
	}

	cpustate->clock = device->clock();

	/* find address spaces */
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	/* allocate timers */
	cpustate->t0_timer = device->machine().scheduler().timer_alloc(FUNC(t0_tick), cpustate);
	cpustate->t1_timer = device->machine().scheduler().timer_alloc(FUNC(t1_tick), cpustate);

	/* register for state saving */
	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->r));
	device->save_item(NAME(cpustate->input));
	device->save_item(NAME(cpustate->output));
	device->save_item(NAME(cpustate->irq));
}

/***************************************************************************
    EXECUTION
***************************************************************************/

static CPU_EXECUTE( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	do
	{
		UINT8 opcode;
		int cycles;

		debugger_instruction_hook(device, cpustate->pc);

		/* TODO: sample interrupts */
		cpustate->input[3] = cpustate->io->read_byte(3);

		/* fetch opcode */
		opcode = fetch(cpustate);
		cycles = Z8601_OPCODE_MAP[opcode].execution_cycles;

		/* execute instruction */
		(*(Z8601_OPCODE_MAP[opcode].function))(cpustate, opcode, &cycles);

		cpustate->icount -= cycles;
	}
	while (cpustate->icount > 0);
}

/***************************************************************************
    RESET
***************************************************************************/

static CPU_RESET( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	cpustate->pc = 0x000c;

	register_write(cpustate, Z8_REGISTER_TMR, 0x00);
	register_write(cpustate, Z8_REGISTER_PRE1, register_read(cpustate, Z8_REGISTER_PRE1) & 0xfc);
	register_write(cpustate, Z8_REGISTER_PRE0, register_read(cpustate, Z8_REGISTER_PRE0) & 0xfe);
	register_write(cpustate, Z8_REGISTER_P2M, 0xff);
	register_write(cpustate, Z8_REGISTER_P3M, 0x00);
	register_write(cpustate, Z8_REGISTER_P01M, 0x4d);
	register_write(cpustate, Z8_REGISTER_IRQ, 0x00);
	register_write(cpustate, Z8_REGISTER_RP, 0x00);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( program_2kb, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_4kb, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case Z8_SP:
		case Z8_GENSP:
			cpustate->r[Z8_REGISTER_SPH] = cpustate->fake_sp >> 8;
			cpustate->r[Z8_REGISTER_SPL] = cpustate->fake_sp & 0xff;
			break;

		case Z8_R0: case Z8_R1: case Z8_R2: case Z8_R3: case Z8_R4: case Z8_R5: case Z8_R6: case Z8_R7: case Z8_R8: case Z8_R9: case Z8_R10: case Z8_R11: case Z8_R12: case Z8_R13: case Z8_R14: case Z8_R15:
			cpustate->r[cpustate->r[Z8_REGISTER_RP] + (entry.index() - Z8_R0)] = cpustate->fake_r[entry.index() - Z8_R0];
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z8) called for unexpected value\n");
			break;
	}
}

static CPU_EXPORT_STATE( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case Z8_SP:
		case Z8_GENSP:
			cpustate->fake_sp = (cpustate->r[Z8_REGISTER_SPH] << 8) | cpustate->r[Z8_REGISTER_SPL];
			break;

		case Z8_R0: case Z8_R1: case Z8_R2: case Z8_R3: case Z8_R4: case Z8_R5: case Z8_R6: case Z8_R7: case Z8_R8: case Z8_R9: case Z8_R10: case Z8_R11: case Z8_R12: case Z8_R13: case Z8_R14: case Z8_R15:
			cpustate->fake_r[entry.index() - Z8_R0] = cpustate->r[cpustate->r[Z8_REGISTER_RP] + (entry.index() - Z8_R0)];
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(z8) called for unexpected value\n");
			break;
	}
}

static CPU_EXPORT_STRING( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS: string.printf("%c%c%c%c%c%c",
									 cpustate->r[Z8_REGISTER_FLAGS] & Z8_FLAGS_C ? 'C' : '.',
									 cpustate->r[Z8_REGISTER_FLAGS] & Z8_FLAGS_Z ? 'Z' : '.',
									 cpustate->r[Z8_REGISTER_FLAGS] & Z8_FLAGS_S ? 'S' : '.',
									 cpustate->r[Z8_REGISTER_FLAGS] & Z8_FLAGS_V ? 'V' : '.',
									 cpustate->r[Z8_REGISTER_FLAGS] & Z8_FLAGS_D ? 'D' : '.',
									 cpustate->r[Z8_REGISTER_FLAGS] & Z8_FLAGS_H ? 'H' : '.');	break;
	}
}

/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

static CPU_SET_INFO( z8 )
{
	z8_state *cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ0:	cpustate->irq[0] = info->i;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ1:	cpustate->irq[1] = info->i;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ2:	cpustate->irq[2] = info->i;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ3:	cpustate->irq[3] = info->i;				break;
	}
}

static CPU_GET_INFO( z8 )
{
	z8_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(z8_state);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 6;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 16;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 16;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 2;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:				info->setinfo = CPU_SET_INFO_NAME(z8);			break;
		case CPUINFO_FCT_INIT:					info->init = CPU_INIT_NAME(z8);					break;
		case CPUINFO_FCT_RESET:					info->reset = CPU_RESET_NAME(z8);				break;
		case CPUINFO_FCT_EXECUTE:				info->execute = CPU_EXECUTE_NAME(z8);			break;
		case CPUINFO_FCT_DISASSEMBLE:			info->disassemble = CPU_DISASSEMBLE_NAME(z8);	break;
		case CPUINFO_FCT_IMPORT_STATE:			info->import_state = CPU_IMPORT_STATE_NAME(z8);	break;
		case CPUINFO_FCT_EXPORT_STATE:			info->export_state = CPU_EXPORT_STATE_NAME(z8);	break;
		case CPUINFO_FCT_EXPORT_STRING:			info->export_string = CPU_EXPORT_STRING_NAME(z8);	break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:	info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Z8");					break;
		case CPUINFO_STR_FAMILY:						strcpy(info->s, "Zilog Z8");			break;
		case CPUINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team"); break;
	}
}

/***************************************************************************
    CPU-SPECIFIC CONTEXT ACCESS
***************************************************************************/

CPU_GET_INFO( z8601 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_2kb);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Z8601");								break;

		default:										CPU_GET_INFO_CALL(z8);									break;
	}
}

CPU_GET_INFO( ub8830d )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_2kb);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "UB8830D");								break;

		default:										CPU_GET_INFO_CALL(z8);									break;
	}
}

CPU_GET_INFO( z8611 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_4kb);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Z8611");								break;

		default:										CPU_GET_INFO_CALL(z8);									break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(Z8601, z8601);
DEFINE_LEGACY_CPU_DEVICE(UB8830D, ub8830d);
DEFINE_LEGACY_CPU_DEVICE(Z8611, z8611);
