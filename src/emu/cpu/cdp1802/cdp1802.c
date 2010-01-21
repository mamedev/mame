#include "emu.h"
#include "debugger.h"
#include "cdp1802.h"

#define CDP1802_CYCLES_RESET		8
#define CDP1802_CYCLES_INIT			8 // really 9, but needs to be 8 to synchronize cdp1861 video timings
#define CDP1802_CYCLES_FETCH		8
#define CDP1802_CYCLES_EXECUTE		8
#define CDP1802_CYCLES_DMA			8
#define CDP1802_CYCLES_INTERRUPT	8

enum _cdp1802_cpu_state
{
	CDP1802_STATE_0_FETCH,
	CDP1802_STATE_1_RESET,
	CDP1802_STATE_1_INIT,
	CDP1802_STATE_1_EXECUTE,
	CDP1802_STATE_2_DMA_IN,
	CDP1802_STATE_2_DMA_OUT,
	CDP1802_STATE_3_INT
};
typedef enum _cdp1802_cpu_state cdp1802_cpu_state;

typedef struct _cdp1802_state cdp1802_state;
struct _cdp1802_state
{
	const cdp1802_interface *intf;

    const address_space *program;
    const address_space *io;

	devcb_resolved_write_line	out_q_func;
	devcb_resolved_read8		in_dma_func;
	devcb_resolved_write8		out_dma_func;

	/* registers */
	UINT8 d;				/* data register (accumulator) */
	int df;					/* data flag (ALU carry) */
	UINT8 b;				/* auxiliary holding register */
	UINT16 r[16];			/* scratchpad registers */
	UINT8 p;				/* designates which register is Program Counter */
	UINT8 x;				/* designates which register is Data Pointer */
	UINT8 n;				/* low-order instruction digit */
	UINT8 i;				/* high-order instruction digit */
	UINT8 t;				/* temporary register */
	int ie;					/* interrupt enable */
	int q;					/* output flip-flop */

	/* cpu state */
	cdp1802_cpu_state state;		/* processor state */
	cdp1802_state_code state_code;	/* state code */
	cdp1802_control_mode mode;		/* control mode */
	cdp1802_control_mode prevmode;	/* previous control mode */

	/* input lines */
	int irq;				/* interrupt request */
	int dmain;				/* DMA input request */
	int dmaout;				/* DMA output request */
	int ef;					/* external flags */

	/* execution logic */
	UINT16 fake_pc;			/* fake program counter */
	int icount;				/* instruction counter */

	cpu_state_table state_table;
};

/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define CDP1802_STATE_ENTRY(_name, _format, _member, _datamask, _flags) \
	CPU_STATE_ENTRY(CDP1802_##_name, #_name, _format, cdp1802_state, _member, _datamask, ~0, _flags)

static const cpu_state_entry state_array[] =
{
	CDP1802_STATE_ENTRY(GENPC, "%04X", fake_pc, 0xffff, CPUSTATE_NOSHOW | CPUSTATE_IMPORT | CPUSTATE_EXPORT)

	CDP1802_STATE_ENTRY(P, "%01X", p, 0xf, 0)
	CDP1802_STATE_ENTRY(X, "%01X", x, 0xf, 0)
	CDP1802_STATE_ENTRY(D, "%02X", d, 0xff, 0)
	CDP1802_STATE_ENTRY(B, "%02X", b, 0xff, 0)
	CDP1802_STATE_ENTRY(T, "%02X", t, 0xff, 0)

	CDP1802_STATE_ENTRY(I, "%01X", i, 0xf, 0)
	CDP1802_STATE_ENTRY(N, "%01X", n, 0xf, 0)

	CDP1802_STATE_ENTRY(R0, "%04X", r[0], 0xffff, 0)
	CDP1802_STATE_ENTRY(R1, "%04X", r[1], 0xffff, 0)
	CDP1802_STATE_ENTRY(R2, "%04X", r[2], 0xffff, 0)
	CDP1802_STATE_ENTRY(R3, "%04X", r[3], 0xffff, 0)
	CDP1802_STATE_ENTRY(R4, "%04X", r[4], 0xffff, 0)
	CDP1802_STATE_ENTRY(R5, "%04X", r[5], 0xffff, 0)
	CDP1802_STATE_ENTRY(R6, "%04X", r[6], 0xffff, 0)
	CDP1802_STATE_ENTRY(R7, "%04X", r[7], 0xffff, 0)
	CDP1802_STATE_ENTRY(R8, "%04X", r[8], 0xffff, 0)
	CDP1802_STATE_ENTRY(R9, "%04X", r[9], 0xffff, 0)
	CDP1802_STATE_ENTRY(Ra, "%04X", r[10], 0xffff, 0)
	CDP1802_STATE_ENTRY(Rb, "%04X", r[11], 0xffff, 0)
	CDP1802_STATE_ENTRY(Rc, "%04X", r[12], 0xffff, 0)
	CDP1802_STATE_ENTRY(Rd, "%04X", r[13], 0xffff, 0)
	CDP1802_STATE_ENTRY(Re, "%04X", r[14], 0xffff, 0)
	CDP1802_STATE_ENTRY(Rf, "%04X", r[15], 0xffff, 0)

	CDP1802_STATE_ENTRY(SC, "%1u", state_code, 0x3, CPUSTATE_NOSHOW)
	CDP1802_STATE_ENTRY(DF, "%1u", df, 0x1, CPUSTATE_NOSHOW)
	CDP1802_STATE_ENTRY(IE, "%1u", ie, 0x1, CPUSTATE_NOSHOW)
	CDP1802_STATE_ENTRY(Q, "%1u", q, 0x1, CPUSTATE_NOSHOW)
};

static const cpu_state_table state_table_template =
{
	NULL,						/* pointer to the base of state (offsets are relative to this) */
	0,							/* subtype this table refers to */
	ARRAY_LENGTH(state_array),	/* number of entries */
	state_array					/* array of entries */
};

INLINE cdp1802_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_CDP1802);
	return (cdp1802_state *)device->token;
}

#define OPCODE_R(addr)		memory_decrypted_read_byte(cpustate->program, addr)
#define RAM_R(addr)			memory_read_byte_8be(cpustate->program, addr)
#define RAM_W(addr, data)	memory_write_byte_8be(cpustate->program, addr, data)
#define IO_R(addr)			memory_read_byte_8be(cpustate->io, addr)
#define IO_W(addr, data)	memory_write_byte_8be(cpustate->io, addr, data)

#define P	cpustate->p
#define X	cpustate->x
#define D	cpustate->d
#define B   cpustate->b
#define T	cpustate->t
#define R   cpustate->r
#define DF	cpustate->df
#define IE	cpustate->ie
#define Q	cpustate->q
#define N	cpustate->n
#define I	cpustate->i

INLINE void cdp1802_add(cdp1802_state *cpustate, int left, int right)
{
	int result = left + right;
	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_add_carry(cdp1802_state *cpustate, int left, int right)
{
	int result = left + right + DF;
	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_sub(cdp1802_state *cpustate, int left, int right)
{
	int result = left + (~right & 0xff) + 1;

	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_sub_carry(cdp1802_state *cpustate, int left, int right)
{
	int result = left + (~right & 0xff) + DF;

	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_short_branch(cdp1802_state *cpustate, int taken)
{
	if (taken)
	{
		R[P] = (R[P] & 0xff00) | OPCODE_R(R[P]);
	}
	else
	{
		R[P] = R[P] + 1;
	}
}

INLINE void cdp1802_long_branch(cdp1802_state *cpustate, int taken)
{
	if (taken)
	{
		// S1#1

		B = OPCODE_R(R[P]);
		R[P] = R[P] + 1;

		// S1#2

		R[P] = (B << 8) | OPCODE_R(R[P]);
	}
	else
	{
		// S1#1

		R[P] = R[P] + 1;

		// S1#2

		R[P] = R[P] + 1;
	}
}

INLINE void cdp1802_long_skip(cdp1802_state *cpustate, int taken)
{
	if (taken)
	{
		// S1#1

		R[P] = R[P] + 1;

		// S1#2

		R[P] = R[P] + 1;
	}
}

static void cdp1802_sample_ef(running_device *device)
{
	cdp1802_state *cpustate = get_safe_token(device);

	if (cpustate->intf->ef_r)
	{
		cpustate->ef = cpustate->intf->ef_r(device) & 0x0f;
	}
	else
	{
		cpustate->ef = 0x0f;
	}
}

static void cdp1802_output_state_code(running_device *device)
{
	cdp1802_state *cpustate = get_safe_token(device);

	if (cpustate->intf->sc_w)
	{
		cdp1802_state_code state_code = CDP1802_STATE_CODE_S0_FETCH;
		int sc0, sc1;

		switch (cpustate->state)
		{
		case CDP1802_STATE_0_FETCH:
			state_code = CDP1802_STATE_CODE_S0_FETCH;
			break;

		case CDP1802_STATE_1_EXECUTE:
		case CDP1802_STATE_1_RESET:
		case CDP1802_STATE_1_INIT:
			state_code = CDP1802_STATE_CODE_S1_EXECUTE;
			break;

		case CDP1802_STATE_2_DMA_IN:
		case CDP1802_STATE_2_DMA_OUT:
			state_code = CDP1802_STATE_CODE_S2_DMA;
			break;

		case CDP1802_STATE_3_INT:
			state_code = CDP1802_STATE_CODE_S3_INTERRUPT;
			break;
		}

		sc0 = BIT(state_code, 0);
		sc1 = BIT(state_code, 1);

		cpustate->intf->sc_w(device, state_code, sc0, sc1);
	}
}

static void cdp1802_run(running_device *device)
{
	cdp1802_state *cpustate = get_safe_token(device);

	switch (cpustate->state)
	{
	case CDP1802_STATE_1_RESET:

		I = 0;
		N = 0;
		Q = 0;
		IE = 1;

		cpustate->icount -= CDP1802_CYCLES_RESET;

		debugger_instruction_hook(device, R[P]);

		break;

	case CDP1802_STATE_1_INIT:

		X = 0;
		P = 0;
		R[0] = 0;

		cpustate->icount -= CDP1802_CYCLES_INIT;

		if (cpustate->dmain)
		{
			cpustate->state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cpustate->dmaout)
		{
			cpustate->state = CDP1802_STATE_2_DMA_OUT;
		}
		else
		{
			cpustate->state = CDP1802_STATE_0_FETCH;
		}

		debugger_instruction_hook(device, R[P]);

		break;

	case CDP1802_STATE_0_FETCH:
		{
		UINT8 opcode = OPCODE_R(R[P]);

		I = opcode >> 4;
		N = opcode & 0x0f;
		R[P] = R[P] + 1;

		cpustate->icount -= CDP1802_CYCLES_FETCH;

		cpustate->state = CDP1802_STATE_1_EXECUTE;
		}
		break;

	case CDP1802_STATE_1_EXECUTE:

		cdp1802_sample_ef(device);

		switch (I)
		{
		case 0:
			if (N > 0)
			{
				D = RAM_R(R[N]);
			}
			break;

		case 1:
			R[N] = R[N] + 1;
			break;

		case 2:
			R[N] = R[N] - 1;
			break;

		case 3:
			switch (N)
			{
			case 0:
				cdp1802_short_branch(cpustate, 1);
				break;

			case 1:
				cdp1802_short_branch(cpustate, Q == 1);
				break;

			case 2:
				cdp1802_short_branch(cpustate, D == 0);
				break;

			case 3:
				cdp1802_short_branch(cpustate, DF == 1);
				break;

			case 4:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF1) ? 0 : 1);
				break;

			case 5:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF2) ? 0 : 1);
				break;

			case 6:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF3) ? 0 : 1);
				break;

			case 7:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF4) ? 0 : 1);
				break;

			case 8:
				cdp1802_short_branch(cpustate, 0);
				break;

			case 9:
				cdp1802_short_branch(cpustate, Q == 0);
				break;

			case 0xa:
				cdp1802_short_branch(cpustate, D != 0);
				break;

			case 0xb:
				cdp1802_short_branch(cpustate, DF == 0);
				break;

			case 0xc:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF1) ? 1 : 0);
				break;

			case 0xd:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF2) ? 1 : 0);
				break;

			case 0xe:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF3) ? 1 : 0);
				break;

			case 0xf:
				cdp1802_short_branch(cpustate, (cpustate->ef & EF4) ? 1 : 0);
				break;
			}
			break;

		case 4:
			D = RAM_R(R[N]);
			R[N] = R[N] + 1;
			break;

		case 5:
			RAM_W(R[N], D);
			break;

		case 6:
			switch (N)
			{
			case 0:
				R[X] = R[X] + 1;
				break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				IO_W(N, RAM_R(R[X]));
				R[X] = R[X] + 1;
				break;

			case 8:
				/*

                    A note about INP 0 (0x68) from Tom Pittman's "A Short Course in Programming":

                    If you look carefully, you will notice that we never studied the opcode "68".
                    That's because it is not a defined 1802 instruction. It has the form of an INP
                    instruction, but 0 is not a defined input port, so if you execute it (try it!)
                    nothing is input. "Nothing" is the answer to a question; it is data, and something
                    will be put in the accumulator and memory (so now you know what the computer uses
                    to mean "nothing").

                    However, since the result of the "68" opcode is unpredictable, it should not be
                    used in your programs. In fact, "68" is the first byte of a series of additional
                    instructions for the 1804 and 1805 microprocessors.

                    http://www.ittybittycomputers.com/IttyBitty/ShortCor.htm

                */
			case 9:
			case 0xa:
			case 0xb:
			case 0xc:
			case 0xd:
			case 0xe:
			case 0xf:
				{
				UINT8 data = IO_R(N & 0x07);
				RAM_W(R[X], data);
				D = data;
				}
				break;
			}
			break;

		case 7:
			switch (N)
			{
			case 0:
				{
				UINT8 data = RAM_R(R[X]);
				R[X] = R[X] + 1;
				P = data & 0xf;
				X = data >> 4;
				IE = 1;
				}
				break;

			case 1:
				{
				UINT8 data = RAM_R(R[X]);
				R[X] = R[X] + 1;
				P = data & 0xf;
				X = data >> 4;
				IE = 0;
				}
				break;

			case 2:
				D = RAM_R(R[X]);
				R[X] = R[X] + 1;
				break;

			case 3:
				RAM_W(R[X], D);
				R[X] = R[X] - 1;
				break;

			case 4:
				cdp1802_add_carry(cpustate, RAM_R(R[X]), D);
				break;

			case 5:
				cdp1802_sub_carry(cpustate, RAM_R(R[X]), D);
				break;

			case 6:
				{
				int b = DF;
				DF = D & 1;
				D >>= 1;
				if (b) D |= 0x80;
				}
				break;

			case 7:
				cdp1802_sub_carry(cpustate, D, RAM_R(R[X]));
				break;

			case 8:
				RAM_W(R[X], T);
				break;

			case 9:
				{
				UINT8 result = (X << 4) | P;
				T = result;
				RAM_W(R[2], result);
				X = P;
				R[2] = R[2] - 1;
				}
				break;

			case 0xa:
				Q = 0;

				devcb_call_write_line(&cpustate->out_q_func, Q);
				break;

			case 0xb:
				Q = 1;

				devcb_call_write_line(&cpustate->out_q_func, Q);
				break;

			case 0xc:
				cdp1802_add_carry(cpustate, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xd:
				cdp1802_sub_carry(cpustate, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xe:
				{
				int b = DF;
				DF = D & 0x80;
				D <<= 1;
				if (b) D |= 1;
				}
				break;

			case 0xf:
				cdp1802_sub_carry(cpustate, D, RAM_R(R[P]));
				R[P] = R[P] + 1;
				break;
			}
			break;

		case 8:
			D = R[N] & 0xff;
			break;

		case 9:
			D = (R[N] >> 8) & 0xff;
			break;

		case 0xa:
			R[N] = (R[N] & 0xff00) | D;
			break;

		case 0xb:
			R[N] = (D << 8) | (R[N] & 0xff);
			break;

		case 0xc:
			cdp1802_output_state_code(device);

			switch (N)
			{
			case 0:
				cdp1802_long_branch(cpustate, 1);
				break;

			case 1:
				cdp1802_long_branch(cpustate, Q == 1);
				break;

			case 2:
				cdp1802_long_branch(cpustate, D == 0);
				break;

			case 3:
				cdp1802_long_branch(cpustate, DF == 1);
				break;

			case 4:
				// NOP
				break;

			case 5:
				cdp1802_long_skip(cpustate, Q == 0);
				break;

			case 6:
				cdp1802_long_skip(cpustate, D != 0);
				break;

			case 7:
				cdp1802_long_skip(cpustate, DF == 0);
				break;

			case 8:
				cdp1802_long_skip(cpustate, 1);
				break;

			case 9:
				cdp1802_long_branch(cpustate, Q == 0);
				break;

			case 0xa:
				cdp1802_long_branch(cpustate, D != 0);
				break;

			case 0xb:
				cdp1802_long_branch(cpustate, DF == 0);
				break;

			case 0xc:
				cdp1802_long_skip(cpustate, IE == 1);
				break;

			case 0xd:
				cdp1802_long_skip(cpustate, Q == 1);
				break;

			case 0xe:
				cdp1802_long_skip(cpustate, D == 0);
				break;

			case 0xf:
				cdp1802_long_skip(cpustate, DF == 1);
				break;
			}

			cpustate->icount -= CDP1802_CYCLES_EXECUTE;
			break;

		case 0xd:
			P = N;
			break;

		case 0xe:
			X = N;
			break;

		case 0xf:
			switch (N)
			{
			case 0:
				D = RAM_R(R[X]);
				break;

			case 1:
				D = RAM_R(R[X]) | D;
				break;

			case 2:
				D = RAM_R(R[X]) & D;
				break;

			case 3:
				D = RAM_R(R[X]) ^ D;
				break;

			case 4:
				cdp1802_add(cpustate, RAM_R(R[X]), D);
				break;

			case 5:
				cdp1802_sub(cpustate, RAM_R(R[X]), D);
				break;

			case 6:
				DF = D & 0x01;
				D = D >> 1;
				break;

			case 7:
				cdp1802_sub(cpustate, D, RAM_R(R[X]));
				break;

			case 8:
				D = RAM_R(R[P]);
				R[P] = R[P] + 1;
				break;

			case 9:
				D = RAM_R(R[P]) | D;
				R[P] = R[P] + 1;
				break;

			case 0xa:
				D = RAM_R(R[P]) & D;
				R[P] = R[P] + 1;
				break;

			case 0xb:
				D = RAM_R(R[P]) ^ D;
				R[P] = R[P] + 1;
				break;

			case 0xc:
				cdp1802_add(cpustate, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xd:
				cdp1802_sub(cpustate, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xe:
				DF = (D & 0x80) >> 7;
				D = D << 1;
				break;

			case 0xf:
				cdp1802_sub(cpustate, D, RAM_R(R[P]));
				R[P] = R[P] + 1;
				break;
			}
			break;
		}

		cpustate->icount -= CDP1802_CYCLES_EXECUTE;

		if (cpustate->dmain)
		{
			cpustate->state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cpustate->dmaout)
		{
			cpustate->state = CDP1802_STATE_2_DMA_OUT;
		}
		else if (IE && cpustate->irq)
		{
			cpustate->state = CDP1802_STATE_3_INT;
		}
		else if ((I > 0) || (N > 0)) // not idling
		{
			cpustate->state = CDP1802_STATE_0_FETCH;
		}

		debugger_instruction_hook(device, R[P]);

		break;

    case CDP1802_STATE_2_DMA_IN:

		RAM_W(R[0], devcb_call_read8(&cpustate->in_dma_func, R[0]));

		R[0] = R[0] + 1;

        cpustate->icount -= CDP1802_CYCLES_DMA;

        if (cpustate->dmain)
        {
            cpustate->state = CDP1802_STATE_2_DMA_IN;
        }
        else if (cpustate->dmaout)
        {
            cpustate->state = CDP1802_STATE_2_DMA_OUT;
        }
        else if (IE && cpustate->irq)
        {
            cpustate->state = CDP1802_STATE_3_INT;
        }
        else if (cpustate->mode == CDP1802_MODE_LOAD)
        {
            cpustate->state = CDP1802_STATE_1_EXECUTE;
        }
        else
        {
            cpustate->state = CDP1802_STATE_0_FETCH;
        }
        break;

    case CDP1802_STATE_2_DMA_OUT:

		devcb_call_write8(&cpustate->out_dma_func, R[0], RAM_R(R[0]));

		R[0] = R[0] + 1;

        cpustate->icount -= CDP1802_CYCLES_DMA;

        if (cpustate->dmain)
        {
            cpustate->state = CDP1802_STATE_2_DMA_IN;
        }
        else if (cpustate->dmaout)
        {
            cpustate->state = CDP1802_STATE_2_DMA_OUT;
        }
        else if (IE && cpustate->irq)
        {
            cpustate->state = CDP1802_STATE_3_INT;
        }
        else
        {
            cpustate->state = CDP1802_STATE_0_FETCH;
        }
        break;

	case CDP1802_STATE_3_INT:

		T = (X << 4) | P;
		X = 2;
		P = 1;
		IE = 0;

		cpustate->icount -= CDP1802_CYCLES_INTERRUPT;

		if (cpustate->dmain)
		{
			cpustate->state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cpustate->dmaout)
		{
			cpustate->state = CDP1802_STATE_2_DMA_OUT;
		}
		else
		{
			cpustate->state = CDP1802_STATE_0_FETCH;
		}

		debugger_instruction_hook(device, R[P]);

		break;
	}
}

static CPU_EXECUTE( cdp1802 )
{
	cdp1802_state *cpustate = get_safe_token(device);

	cpustate->icount = cycles;

	cpustate->prevmode = cpustate->mode;
	cpustate->mode = cpustate->intf->mode_r(device);

	do
	{
		switch (cpustate->mode)
		{
		case CDP1802_MODE_LOAD:
			if (cpustate->prevmode == CDP1802_MODE_RESET)
			{
				cpustate->prevmode = CDP1802_MODE_LOAD;

				/* execute initialization cycle */
				cpustate->state = CDP1802_STATE_1_INIT;
				cdp1802_run(device);

				/* next state is IDLE */
				cpustate->state = CDP1802_STATE_1_EXECUTE;
			}
			else
			{
				/* idle */
				I = 0;
				N = 0;
				cdp1802_run(device);
			}
			break;

		case CDP1802_MODE_RESET:
			cpustate->state = CDP1802_STATE_1_RESET;
			cdp1802_run(device);
			break;

		case CDP1802_MODE_PAUSE:
			cpustate->icount -= 1;
			break;

		case CDP1802_MODE_RUN:
			switch (cpustate->prevmode)
			{
			case CDP1802_MODE_LOAD:
				// RUN mode cannot be initiated from LOAD mode
				logerror("CDP1802 '%s' Tried to initiate RUN mode from LOAD mode\n", device->tag.cstr());
				cpustate->mode = CDP1802_MODE_LOAD;
				break;

			case CDP1802_MODE_RESET:
				cpustate->prevmode = CDP1802_MODE_RUN;
				cpustate->state = CDP1802_STATE_1_INIT;
				cdp1802_run(device);
				break;

			case CDP1802_MODE_PAUSE:
				cpustate->prevmode = CDP1802_MODE_RUN;
				cpustate->state = CDP1802_STATE_0_FETCH;
				cdp1802_run(device);
				break;

			case CDP1802_MODE_RUN:
				cdp1802_run(device);
				break;
			}
			break;
		}

		cdp1802_output_state_code(device);
	}
	while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

static CPU_RESET( cdp1802 )
{
	cdp1802_state *cpustate = get_safe_token(device);

	cpustate->mode = CDP1802_MODE_RESET;
}

static CPU_INIT( cdp1802 )
{
	cdp1802_state *cpustate = get_safe_token(device);
	int i;

	cpustate->intf = (cdp1802_interface *) device->baseconfig().static_config;

	/* resolve callbacks */
	devcb_resolve_write_line(&cpustate->out_q_func, &cpustate->intf->out_q_func, device);
	devcb_resolve_read8(&cpustate->in_dma_func, &cpustate->intf->in_dma_func, device);
	devcb_resolve_write8(&cpustate->out_dma_func, &cpustate->intf->out_dma_func, device);

	/* set up the state table */
	cpustate->state_table = state_table_template;
	cpustate->state_table.baseptr = cpustate;
	cpustate->state_table.subtypemask = 1;

	/* find address spaces */
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	/* set initial values */
	cpustate->p = mame_rand(device->machine) & 0x0f;
	cpustate->x = mame_rand(device->machine) & 0x0f;
	cpustate->d = mame_rand(device->machine);
	cpustate->b = mame_rand(device->machine);
	cpustate->t = mame_rand(device->machine);
	cpustate->n = mame_rand(device->machine) & 0x0f;
	cpustate->i = mame_rand(device->machine) & 0x0f;

	for (i = 0; i < 16; i++)
	{
		cpustate->r[i] = mame_rand(device->machine);
	}

	cpustate->mode = CDP1802_MODE_RESET;
	cpustate->prevmode = cpustate->mode;
	cpustate->irq = CLEAR_LINE;
	cpustate->dmain = CLEAR_LINE;
	cpustate->dmaout = CLEAR_LINE;

	/* register for state saving */
	state_save_register_device_item(device, 0, cpustate->p);
	state_save_register_device_item(device, 0, cpustate->x);
	state_save_register_device_item(device, 0, cpustate->d);
	state_save_register_device_item(device, 0, cpustate->b);
	state_save_register_device_item(device, 0, cpustate->t);
	state_save_register_device_item_array(device, 0, cpustate->r);
	state_save_register_device_item(device, 0, cpustate->df);
	state_save_register_device_item(device, 0, cpustate->ie);
	state_save_register_device_item(device, 0, cpustate->q);
	state_save_register_device_item(device, 0, cpustate->n);
	state_save_register_device_item(device, 0, cpustate->i);
	state_save_register_device_item(device, 0, cpustate->state);
	state_save_register_device_item(device, 0, cpustate->prevmode);
	state_save_register_device_item(device, 0, cpustate->mode);
	state_save_register_device_item(device, 0, cpustate->irq);
	state_save_register_device_item(device, 0, cpustate->dmain);
	state_save_register_device_item(device, 0, cpustate->dmaout);
	state_save_register_device_item(device, 0, cpustate->ef);
}

/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( cdp1802 )
{
	cdp1802_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case CDP1802_GENPC:
			R[P] = cpustate->fake_pc;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(cdp1802) called for unexpected value\n");
			break;
	}
}

static CPU_EXPORT_STATE( cdp1802 )
{
	cdp1802_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case CDP1802_GENPC:
			cpustate->fake_pc = R[P];
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(cdp1802) called for unexpected value\n");
			break;
	}
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( cdp1802 )
{
	cdp1802_state *cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_INT:		cpustate->irq = info->i;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAIN:	cpustate->dmain = info->i;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAOUT:	cpustate->dmaout = info->i;	break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( cdp1802 )
{
	cdp1802_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cdp1802_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = CDP1802_CYCLES_EXECUTE * 2;	break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = CDP1802_CYCLES_EXECUTE * 3;	break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;									break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;									break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;									break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;									break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;									break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;									break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;									break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 3;									break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;									break;

		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_INT:		info->i = cpustate->irq;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAIN:	info->i = cpustate->dmain;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAOUT:	info->i = cpustate->dmaout;	break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cdp1802);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cdp1802);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(cdp1802);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cdp1802);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cdp1802);	break;
		case CPUINFO_FCT_IMPORT_STATE:					info->import_state = CPU_IMPORT_STATE_NAME(cdp1802);break;
		case CPUINFO_FCT_EXPORT_STATE:					info->export_state = CPU_EXPORT_STATE_NAME(cdp1802);break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;
		case CPUINFO_PTR_STATE_TABLE:					info->state_table = &cpustate->state_table;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "CDP1802");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "RCA COSMAC");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

		case CPUINFO_STR_FLAGS: sprintf(info->s,
									"%c%c%c",
									 cpustate->df ? 'D' : '.',
									 cpustate->ie ? 'I' : '.',
									 cpustate->q ? 'Q' : '.'); break;
	}
}
