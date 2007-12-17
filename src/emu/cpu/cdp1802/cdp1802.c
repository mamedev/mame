#include "driver.h"
#include "state.h"
#include "debugger.h"
#include "cdp1802.h"

typedef struct
{
	CDP1802_CONFIG *config;

	UINT8 p, x, d, b, t;
	UINT16 r[16];
	UINT8 df, ie, q, n, i;

	int state;
	int prevmode, mode;
	int irq, dmain, dmaout;
	int ef;

} CDP1802_Regs;

#define M	program_read_byte
#define MW	program_write_byte

#define P	cdp1802.p
#define X	cdp1802.x
#define D	cdp1802.d
#define B   cdp1802.b
#define T	cdp1802.t
#define R   cdp1802.r
#define DF	cdp1802.df
#define IE	cdp1802.ie
#define Q	cdp1802.q
#define N	cdp1802.n
#define I	cdp1802.i

static int cdp1802_ICount;

static CDP1802_Regs cdp1802;

static void cdp1802_get_context (void *dst)
{
	*(CDP1802_Regs *)dst = cdp1802;
}

static void cdp1802_set_context (void *src)
{
	cdp1802 = *(CDP1802_Regs *)src;
}

static void cdp1802_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	cdp1802.config = (CDP1802_CONFIG *) config;

	cdp1802.mode = CDP1802_MODE_RESET;
	cdp1802.prevmode = cdp1802.mode;
	cdp1802.irq = CLEAR_LINE;
	cdp1802.dmain = CLEAR_LINE;
	cdp1802.dmaout = CLEAR_LINE;

	state_save_register_item("cdp1802", index, cdp1802.p);
	state_save_register_item("cdp1802", index, cdp1802.x);
	state_save_register_item("cdp1802", index, cdp1802.d);
	state_save_register_item("cdp1802", index, cdp1802.b);
	state_save_register_item("cdp1802", index, cdp1802.t);
	state_save_register_item_array("cdp1802", index, cdp1802.r);
	state_save_register_item("cdp1802", index, cdp1802.df);
	state_save_register_item("cdp1802", index, cdp1802.ie);
	state_save_register_item("cdp1802", index, cdp1802.q);
	state_save_register_item("cdp1802", index, cdp1802.n);
	state_save_register_item("cdp1802", index, cdp1802.i);
	state_save_register_item("cdp1802", index, cdp1802.state);
	state_save_register_item("cdp1802", index, cdp1802.prevmode);
	state_save_register_item("cdp1802", index, cdp1802.mode);
	state_save_register_item("cdp1802", index, cdp1802.irq);
	state_save_register_item("cdp1802", index, cdp1802.dmain);
	state_save_register_item("cdp1802", index, cdp1802.dmaout);
	state_save_register_item("cdp1802", index, cdp1802.ef);
}

INLINE void cdp1802_add(int left, int right)
{
	int result = left + right;
	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_add_carry(int left, int right)
{
	int result = left + right + DF;
	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_sub(int left, int right)
{
	int result = left + (~right & 0xff) + 1;

	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_sub_carry(int left, int right)
{
	int result = left + (~right & 0xff) + DF;

	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_short_branch(int taken)
{
	if (taken)
	{
		R[P] = (R[P] & 0xff00) | cpu_readop(R[P]);
	}
	else
	{
		R[P] = R[P] + 1;
	}
}

INLINE void cdp1802_long_branch(int taken)
{
	if (taken)
	{
		// S1#1

		B = cpu_readop(R[P]);
		R[P] = R[P] + 1;

		// S1#2

		R[P] = (B << 8) | cpu_readop(R[P]);
	}
	else
	{
		// S1#1

		R[P] = R[P] + 1;

		// S1#2

		R[P] = R[P] + 1;
	}
}

INLINE void cdp1802_long_skip(int taken)
{
	if (taken)
	{
		// S1#1

		R[P] = R[P] + 1;

		// S1#2

		R[P] = R[P] + 1;
	}
}

static void cdp1802_sample_ef(void)
{
	if (cdp1802.config->ef_r)
	{
		cdp1802.ef = cdp1802.config->ef_r() & 0x0f;
	}
	else
	{
		cdp1802.ef = 0x0f;
	}
}

static void cdp1802_output_state_code(void)
{
	if (cdp1802.config->sc_w)
	{
		switch (cdp1802.state)
		{
		case CDP1802_STATE_0_FETCH:
			cdp1802.config->sc_w(CDP1802_STATE_CODE_S0_FETCH);
			break;

		case CDP1802_STATE_1_EXECUTE:
			cdp1802.config->sc_w(CDP1802_STATE_CODE_S1_EXECUTE);
			break;

		case CDP1802_STATE_2_DMA_IN:
		case CDP1802_STATE_2_DMA_OUT:
			cdp1802.config->sc_w(CDP1802_STATE_CODE_S2_DMA);
			break;

		case CDP1802_STATE_3_INT:
			cdp1802.config->sc_w(CDP1802_STATE_CODE_S3_INTERRUPT);
			break;
		}
	}
}

static void cdp1802_run(void)
{
	cdp1802_output_state_code();

	switch (cdp1802.state)
	{
	case CDP1802_STATE_1_RESET:

		I = 0;
		N = 0;
		Q = 0;
		IE = 1;

		cdp1802_ICount -= CDP1802_CYCLES_RESET;

		CALL_MAME_DEBUG;

		break;

	case CDP1802_STATE_1_INIT:

		X = 0;
		P = 0;
		R[0] = 0;

		cdp1802_ICount -= CDP1802_CYCLES_INIT;

		if (cdp1802.dmain)
		{
			cdp1802.state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cdp1802.dmaout)
		{
			cdp1802.state = CDP1802_STATE_2_DMA_OUT;
		}
		else
		{
			cdp1802.state = CDP1802_STATE_0_FETCH;
		}

		CALL_MAME_DEBUG;

		break;

	case CDP1802_STATE_0_FETCH:
		{
		UINT8 opcode = cpu_readop(R[P]);

		I = opcode >> 4;
		N = opcode & 0x0f;
		R[P] = R[P] + 1;

		cdp1802_ICount -= CDP1802_CYCLES_FETCH;

		cdp1802.state = CDP1802_STATE_1_EXECUTE;
		}
		break;

	case CDP1802_STATE_1_EXECUTE:

		cdp1802_sample_ef();

		switch (I)
		{
		case 0:
			if (N > 0)
			{
				D = M(R[N]);
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
				cdp1802_short_branch(1);
				break;

			case 1:
				cdp1802_short_branch(Q == 1);
				break;

			case 2:
				cdp1802_short_branch(D == 0);
				break;

			case 3:
				cdp1802_short_branch(DF == 1);
				break;

			case 4:
				cdp1802_short_branch((cdp1802.ef & EF1) ? 0 : 1);
				break;

			case 5:
				cdp1802_short_branch((cdp1802.ef & EF2) ? 0 : 1);
				break;

			case 6:
				cdp1802_short_branch((cdp1802.ef & EF3) ? 0 : 1);
				break;

			case 7:
				cdp1802_short_branch((cdp1802.ef & EF4) ? 0 : 1);
				break;

			case 8:
				cdp1802_short_branch(0);
				break;

			case 9:
				cdp1802_short_branch(Q == 0);
				break;

			case 0xa:
				cdp1802_short_branch(D != 0);
				break;

			case 0xb:
				cdp1802_short_branch(DF == 0);
				break;

			case 0xc:
				cdp1802_short_branch((cdp1802.ef & EF1) ? 1 : 0);
				break;

			case 0xd:
				cdp1802_short_branch((cdp1802.ef & EF2) ? 1 : 0);
				break;

			case 0xe:
				cdp1802_short_branch((cdp1802.ef & EF3) ? 1 : 0);
				break;

			case 0xf:
				cdp1802_short_branch((cdp1802.ef & EF4) ? 1 : 0);
				break;
			}
			break;

		case 4:
			D = M(R[N]);
			R[N] = R[N] + 1;
			break;

		case 5:
			MW(R[N], D);
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
				io_write_byte(N, M(R[X]));
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
				UINT8 data = io_read_byte(N & 0x07);
				MW(R[X], data);
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
				UINT8 data = M(R[X]);
				R[X] = R[X] + 1;
				P = data & 0xf;
				X = data >> 4;
				IE = 1;
				}
				break;

			case 1:
				{
				UINT8 data = M(R[X]);
				R[X] = R[X] + 1;
				P = data & 0xf;
				X = data >> 4;
				IE = 0;
				}
				break;

			case 2:
				D = M(R[X]);
				R[X] = R[X] + 1;
				break;

			case 3:
				MW(R[X], D);
				R[X] = R[X] - 1;
				break;

			case 4:
				cdp1802_add_carry(M(R[X]), D);
				break;

			case 5:
				cdp1802_sub_carry(M(R[X]), D);
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
				cdp1802_sub_carry(D, M(R[X]));
				break;

			case 8:
				MW(R[X], T);
				break;

			case 9:
				{
				UINT8 result = (X << 4) | P;
				T = result;
				MW(R[2], result);
				X = P;
				R[2] = R[2] - 1;
				}
				break;

			case 0xa:
				Q = 0;

				if (cdp1802.config->q_w)
				{
					cdp1802.config->q_w(Q);
				}
				break;

			case 0xb:
				Q = 1;

				if (cdp1802.config->q_w)
				{
					cdp1802.config->q_w(Q);
				}
				break;

			case 0xc:
				cdp1802_add_carry(M(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xd:
				cdp1802_sub_carry(M(R[P]), D);
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
				cdp1802_sub_carry(D, M(R[P]));
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
			cdp1802_output_state_code();

			switch (N)
			{
			case 0:
				cdp1802_long_branch(1);
				break;

			case 1:
				cdp1802_long_branch(Q == 1);
				break;

			case 2:
				cdp1802_long_branch(D == 0);
				break;

			case 3:
				cdp1802_long_branch(DF == 1);
				break;

			case 4:
				// NOP
				break;

			case 5:
				cdp1802_long_skip(Q == 0);
				break;

			case 6:
				cdp1802_long_skip(D != 0);
				break;

			case 7:
				cdp1802_long_skip(DF == 0);
				break;

			case 8:
				cdp1802_long_skip(1);
				break;

			case 9:
				cdp1802_long_branch(Q == 0);
				break;

			case 0xa:
				cdp1802_long_branch(D != 0);
				break;

			case 0xb:
				cdp1802_long_branch(DF == 0);
				break;

			case 0xc:
				cdp1802_long_skip(IE == 1);
				break;

			case 0xd:
				cdp1802_long_skip(Q == 1);
				break;

			case 0xe:
				cdp1802_long_skip(D == 0);
				break;

			case 0xf:
				cdp1802_long_skip(DF == 1);
				break;
			}

			cdp1802_ICount -= CDP1802_CYCLES_EXECUTE;
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
				D = M(R[X]);
				break;

			case 1:
				D = M(R[X]) | D;
				break;

			case 2:
				D = M(R[X]) & D;
				break;

			case 3:
				D = M(R[X]) ^ D;
				break;

			case 4:
				cdp1802_add(M(R[X]), D);
				break;

			case 5:
				cdp1802_sub(M(R[X]), D);
				break;

			case 6:
				DF = D & 0x01;
				D = D >> 1;
				break;

			case 7:
				cdp1802_sub(D, M(R[X]));
				break;

			case 8:
				D = M(R[P]);
				R[P] = R[P] + 1;
				break;

			case 9:
				D = M(R[P]) | D;
				R[P] = R[P] + 1;
				break;

			case 0xa:
				D = M(R[P]) & D;
				R[P] = R[P] + 1;
				break;

			case 0xb:
				D = M(R[P]) ^ D;
				R[P] = R[P] + 1;
				break;

			case 0xc:
				cdp1802_add(M(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xd:
				cdp1802_sub(M(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xe:
				DF = (D & 0x80) >> 7;
				D = D << 1;
				break;

			case 0xf:
				cdp1802_sub(D, M(R[P]));
				R[P] = R[P] + 1;
				break;
			}
			break;
		}

		cdp1802_ICount -= CDP1802_CYCLES_EXECUTE;

		if (cdp1802.dmain)
		{
			cdp1802.state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cdp1802.dmaout)
		{
			cdp1802.state = CDP1802_STATE_2_DMA_OUT;
		}
		else if (IE && cdp1802.irq)
		{
			cdp1802.state = CDP1802_STATE_3_INT;
		}
		else if ((I > 0) || (N > 0)) // not idling
		{
			cdp1802.state = CDP1802_STATE_0_FETCH;
		}

		CALL_MAME_DEBUG;

		break;

    case CDP1802_STATE_2_DMA_IN:

		if (cdp1802.config->dma_r)
		{
			MW(R[0], cdp1802.config->dma_r());
		}

		R[0] = R[0] + 1;

        cdp1802_ICount -= CDP1802_CYCLES_DMA;

        if (cdp1802.dmain)
        {
            cdp1802.state = CDP1802_STATE_2_DMA_IN;
        }
        else if (cdp1802.dmaout)
        {
            cdp1802.state = CDP1802_STATE_2_DMA_OUT;
        }
        else if (IE && cdp1802.irq)
        {
            cdp1802.state = CDP1802_STATE_3_INT;
        }
        else if (cdp1802.mode == CDP1802_MODE_LOAD)
        {
            cdp1802.state = CDP1802_STATE_1_EXECUTE;
        }
        else
        {
            cdp1802.state = CDP1802_STATE_0_FETCH;
        }
        break;

    case CDP1802_STATE_2_DMA_OUT:

		if (cdp1802.config->dma_w)
		{
	        cdp1802.config->dma_w(M(R[0]));
		}

		R[0] = R[0] + 1;

        cdp1802_ICount -= CDP1802_CYCLES_DMA;

        if (cdp1802.dmain)
        {
            cdp1802.state = CDP1802_STATE_2_DMA_IN;
        }
        else if (cdp1802.dmaout)
        {
            cdp1802.state = CDP1802_STATE_2_DMA_OUT;
        }
        else if (IE && cdp1802.irq)
        {
            cdp1802.state = CDP1802_STATE_3_INT;
        }
        else
        {
            cdp1802.state = CDP1802_STATE_0_FETCH;
        }
        break;

	case CDP1802_STATE_3_INT:

		T = (X << 4) | P;
		X = 2;
		P = 1;
		IE = 0;

		cdp1802_ICount -= CDP1802_CYCLES_INTERRUPT;

		if (cdp1802.dmain)
		{
			cdp1802.state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cdp1802.dmaout)
		{
			cdp1802.state = CDP1802_STATE_2_DMA_OUT;
		}
		else
		{
			cdp1802.state = CDP1802_STATE_0_FETCH;
		}

		CALL_MAME_DEBUG;

		break;
	}
}

static int cdp1802_execute(int cycles)
{
	cdp1802_ICount = cycles;

	cdp1802.prevmode = cdp1802.mode;
	cdp1802.mode = cdp1802.config->mode_r();

	do
	{
		switch (cdp1802.mode)
		{
		case CDP1802_MODE_LOAD:
			I = 0;
			N = 0;
			cdp1802.state = CDP1802_STATE_1_EXECUTE;
			cdp1802_run();
			break;

		case CDP1802_MODE_RESET:
			cdp1802.state = CDP1802_STATE_1_RESET;
			cdp1802_run();
			break;

		case CDP1802_MODE_PAUSE:
			cdp1802_ICount -= 1;
			break;

		case CDP1802_MODE_RUN:
			switch (cdp1802.prevmode)
			{
			case CDP1802_MODE_LOAD:
				// RUN mode cannot be initiated from LOAD mode
				cdp1802.mode = CDP1802_MODE_LOAD;
				break;

			case CDP1802_MODE_RESET:
				cdp1802.prevmode = CDP1802_MODE_RUN;
				cdp1802.state = CDP1802_STATE_1_INIT;
				cdp1802_run();
				break;

			case CDP1802_MODE_PAUSE:
				cdp1802.prevmode = CDP1802_MODE_RUN;
				cdp1802.state = CDP1802_STATE_0_FETCH;
				cdp1802_run();
				break;

			case CDP1802_MODE_RUN:
				cdp1802_run();
				break;
			}
			break;
		}
	}
	while (cdp1802_ICount > 0);

	return cycles - cdp1802_ICount;
}

static void cdp1802_set_interrupt_line(int state)
{
	cdp1802.irq = state;
}

static void cdp1802_set_dmain_line(int state)
{
	cdp1802.dmain = state;
}

static void cdp1802_set_dmaout_line(int state)
{
	cdp1802.dmaout = state;
}

static void cdp1802_reset(void)
{
	cdp1802.mode = CDP1802_MODE_RESET;
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void cdp1802_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_INT:		cdp1802_set_interrupt_line(info->i);	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAIN:	cdp1802_set_dmain_line(info->i);		break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAOUT:	cdp1802_set_dmaout_line(info->i);		break;

		case CPUINFO_INT_REGISTER + CDP1802_P:			cdp1802.p = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_X:			cdp1802.x = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_T:			cdp1802.t = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_D:			cdp1802.d = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_B:			cdp1802.b = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R0:			cdp1802.r[0] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R1:			cdp1802.r[1] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R2:			cdp1802.r[2] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R3:			cdp1802.r[3] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R4:			cdp1802.r[4] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R5:			cdp1802.r[5] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R6:			cdp1802.r[6] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R7:			cdp1802.r[7] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R8:			cdp1802.r[8] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R9:			cdp1802.r[9] = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_Ra:			cdp1802.r[0xa] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rb:			cdp1802.r[0xb] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rc:			cdp1802.r[0xc] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rd:			cdp1802.r[0xd] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Re:			cdp1802.r[0xe] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rf:			cdp1802.r[0xf] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_DF:			cdp1802.df = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_IE:			cdp1802.ie = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_Q:			cdp1802.q = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_N:			cdp1802.n = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_I:			cdp1802.i = info->i;		break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CDP1802_PC: 		cdp1802.r[cdp1802.p] = info->i;	break;
	}
}

/****************************************************************************
 * Return a formatted string for a register
 ****************************************************************************/
void cdp1802_get_info(UINT32 state, cpuinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cdp1802);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = CDP1802_CYCLES_EXECUTE * 2;	break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = CDP1802_CYCLES_EXECUTE * 3;	break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 3;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_INT:		info->i = cdp1802.irq;		break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAIN:	info->i = cdp1802.dmain;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAOUT:	info->i = cdp1802.dmaout;	break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_SP:							info->i = 0;							break;
		case CPUINFO_INT_REGISTER + CDP1802_P:			info->i = cdp1802.p;					break;
		case CPUINFO_INT_REGISTER + CDP1802_X:			info->i = cdp1802.x;					break;
		case CPUINFO_INT_REGISTER + CDP1802_T:			info->i = cdp1802.t;					break;
		case CPUINFO_INT_REGISTER + CDP1802_D:			info->i = cdp1802.d;					break;
		case CPUINFO_INT_REGISTER + CDP1802_B:			info->i = cdp1802.b;					break;
		case CPUINFO_INT_REGISTER + CDP1802_R0:			info->i = cdp1802.r[0];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R1:			info->i = cdp1802.r[1];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R2:			info->i = cdp1802.r[2];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R3:			info->i = cdp1802.r[3];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R4:			info->i = cdp1802.r[4];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R5:			info->i = cdp1802.r[5];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R6:			info->i = cdp1802.r[6];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R7:			info->i = cdp1802.r[7];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R8:			info->i = cdp1802.r[8];					break;
		case CPUINFO_INT_REGISTER + CDP1802_R9:			info->i = cdp1802.r[9];					break;
		case CPUINFO_INT_REGISTER + CDP1802_Ra:			info->i = cdp1802.r[0xa];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rb:			info->i = cdp1802.r[0xb];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rc:			info->i = cdp1802.r[0xc];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rd:			info->i = cdp1802.r[0xd];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Re:			info->i = cdp1802.r[0xe];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rf:			info->i = cdp1802.r[0xf];				break;
		case CPUINFO_INT_REGISTER + CDP1802_DF:			info->i = cdp1802.df;					break;
		case CPUINFO_INT_REGISTER + CDP1802_IE:			info->i = cdp1802.ie;					break;
		case CPUINFO_INT_REGISTER + CDP1802_Q:			info->i = cdp1802.q;					break;
		case CPUINFO_INT_REGISTER + CDP1802_N:			info->i = cdp1802.n;					break;
		case CPUINFO_INT_REGISTER + CDP1802_I:			info->i = cdp1802.i;					break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CDP1802_PC:			info->i = cdp1802.r[cdp1802.p];			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = cdp1802_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = cdp1802_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = cdp1802_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = cdp1802_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = cdp1802_reset;			break;
		case CPUINFO_PTR_EXECUTE:						info->execute = cdp1802_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = cdp1802_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cdp1802_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "CDP1802");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "CDP1800");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 2007 The MAME team"); break;

		case CPUINFO_STR_REGISTER + CDP1802_PC:	sprintf(info->s, "PC:%.4x", cdp1802.r[cdp1802.p]);	break;
		case CPUINFO_STR_REGISTER + CDP1802_R0:	sprintf(info->s, "R0:%.4x", cdp1802.r[0]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R1:	sprintf(info->s, "R1:%.4x", cdp1802.r[1]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R2:	sprintf(info->s, "R2:%.4x", cdp1802.r[2]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R3:	sprintf(info->s, "R3:%.4x", cdp1802.r[3]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R4:	sprintf(info->s, "R4:%.4x", cdp1802.r[4]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R5:	sprintf(info->s, "R5:%.4x", cdp1802.r[5]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R6:	sprintf(info->s, "R6:%.4x", cdp1802.r[6]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R7:	sprintf(info->s, "R7:%.4x", cdp1802.r[7]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R8:	sprintf(info->s, "R8:%.4x", cdp1802.r[8]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R9:	sprintf(info->s, "R9:%.4x", cdp1802.r[9]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Ra:	sprintf(info->s, "Ra:%.4x", cdp1802.r[0xa]); 	break;
		case CPUINFO_STR_REGISTER + CDP1802_Rb:	sprintf(info->s, "Rb:%.4x", cdp1802.r[0xb]); 	break;
		case CPUINFO_STR_REGISTER + CDP1802_Rc:	sprintf(info->s, "Rc:%.4x", cdp1802.r[0xc]); 	break;
		case CPUINFO_STR_REGISTER + CDP1802_Rd:	sprintf(info->s, "Rd:%.4x", cdp1802.r[0xd]); 	break;
		case CPUINFO_STR_REGISTER + CDP1802_Re:	sprintf(info->s, "Re:%.4x", cdp1802.r[0xe]); 	break;
		case CPUINFO_STR_REGISTER + CDP1802_Rf:	sprintf(info->s, "Rf:%.4x", cdp1802.r[0xf]); 	break;
		case CPUINFO_STR_REGISTER + CDP1802_P:	sprintf(info->s, "P:%x",    cdp1802.p);			break;
		case CPUINFO_STR_REGISTER + CDP1802_X:	sprintf(info->s, "X:%x",    cdp1802.x);			break;
		case CPUINFO_STR_REGISTER + CDP1802_D:	sprintf(info->s, "D:%.2x",  cdp1802.d);			break;
		case CPUINFO_STR_REGISTER + CDP1802_B:	sprintf(info->s, "B:%.2x",  cdp1802.b);			break;
		case CPUINFO_STR_REGISTER + CDP1802_T:	sprintf(info->s, "T:%.2x",  cdp1802.t);			break;
		case CPUINFO_STR_REGISTER + CDP1802_DF:	sprintf(info->s, "DF:%x",   cdp1802.df);		break;
		case CPUINFO_STR_REGISTER + CDP1802_IE:	sprintf(info->s, "IE:%x",   cdp1802.ie);		break;
		case CPUINFO_STR_REGISTER + CDP1802_Q:	sprintf(info->s, "Q:%x",    cdp1802.q);			break;
		case CPUINFO_STR_REGISTER + CDP1802_N:	sprintf(info->s, "N:%x",    cdp1802.n);			break;
		case CPUINFO_STR_REGISTER + CDP1802_I:	sprintf(info->s, "I:%x",    cdp1802.i);			break;
		case CPUINFO_STR_FLAGS: sprintf(info->s,
									"%s%s%s",
									 cdp1802.df ? "DF" : "..",
									 cdp1802.ie ? "IE" : "..",
									 cdp1802.q ? "Q" : "."); break;
	}
}
