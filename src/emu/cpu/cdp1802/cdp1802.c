#define NO_LEGACY_MEMORY_HANDLERS    1

#include "driver.h"
#include "debugger.h"
#include "cdp1802.h"

#define CDP1802_CYCLES_RESET 		8
#define CDP1802_CYCLES_INIT			8 // really 9, but needs to be 8 to synchronize cdp1861 video timings
#define CDP1802_CYCLES_FETCH		8
#define CDP1802_CYCLES_EXECUTE		8
#define CDP1802_CYCLES_DMA			8
#define CDP1802_CYCLES_INTERRUPT	8

typedef enum _cdp1802_cpu_state cdp1802_cpu_state;
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

typedef struct _cdp1802_state cdp1802_state;
struct _cdp1802_state
{
	const cdp1802_interface *intf;

    const address_space *program;
    const address_space *io;

	/* registers */
	UINT8 d;				/* data register (accumulator) */
	UINT8 df;				/* data flag (ALU carry) */
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
	cdp1802_control_mode mode;		/* control mode */
	cdp1802_control_mode prevmode;	/* previous control mode */

	/* input lines */
	int irq;				/* interrupt request */
	int dmain;				/* DMA input request */
	int dmaout;				/* DMA output request */
	int ef;					/* external flags */

	/* execution logic */
	int icount;				/* instruction counter */
};

#define OPCODE_R(addr)		memory_decrypted_read_byte(cdp1802->program, addr)
#define RAM_R(addr)			memory_read_byte_8le(cdp1802->program, addr)
#define RAM_W(addr, data)	memory_write_byte_8le(cdp1802->program, addr, data)
#define IO_R(addr)			memory_read_byte_8le(cdp1802->io, addr)
#define IO_W(addr, data)	memory_write_byte_8le(cdp1802->io, addr, data)

#define P	cdp1802->p
#define X	cdp1802->x
#define D	cdp1802->d
#define B   cdp1802->b
#define T	cdp1802->t
#define R   cdp1802->r
#define DF	cdp1802->df
#define IE	cdp1802->ie
#define Q	cdp1802->q
#define N	cdp1802->n
#define I	cdp1802->i

INLINE void cdp1802_add(cdp1802_state *cdp1802, int left, int right)
{
	int result = left + right;
	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_add_carry(cdp1802_state *cdp1802, int left, int right)
{
	int result = left + right + DF;
	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_sub(cdp1802_state *cdp1802, int left, int right)
{
	int result = left + (~right & 0xff) + 1;

	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_sub_carry(cdp1802_state *cdp1802, int left, int right)
{
	int result = left + (~right & 0xff) + DF;

	D = result & 0xff;
	DF = (result & 0x100) >> 8;
}

INLINE void cdp1802_short_branch(cdp1802_state *cdp1802, int taken)
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

INLINE void cdp1802_long_branch(cdp1802_state *cdp1802, int taken)
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

INLINE void cdp1802_long_skip(cdp1802_state *cdp1802, int taken)
{
	if (taken)
	{
		// S1#1

		R[P] = R[P] + 1;

		// S1#2

		R[P] = R[P] + 1;
	}
}

static void cdp1802_sample_ef(const device_config *device)
{
	cdp1802_state *cdp1802 = device->token;

	if (cdp1802->intf->ef_r)
	{
		cdp1802->ef = cdp1802->intf->ef_r(device) & 0x0f;
	}
	else
	{
		cdp1802->ef = 0x0f;
	}
}

static void cdp1802_output_state_code(const device_config *device)
{
	cdp1802_state *cdp1802 = device->token;

	if (cdp1802->intf->sc_w)
	{
		cdp1802_state_code state_code = CDP1802_STATE_CODE_S0_FETCH;

		switch (cdp1802->state)
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

		cdp1802->intf->sc_w(device, state_code);
	}
}

static void cdp1802_run(const device_config *device)
{
	cdp1802_state *cdp1802 = device->token;

	cdp1802_output_state_code(device);

	switch (cdp1802->state)
	{
	case CDP1802_STATE_1_RESET:

		I = 0;
		N = 0;
		Q = 0;
		IE = 1;

		cdp1802->icount -= CDP1802_CYCLES_RESET;

		debugger_instruction_hook(device, cdp1802->r[cdp1802->p]);

		break;

	case CDP1802_STATE_1_INIT:

		X = 0;
		P = 0;
		R[0] = 0;

		cdp1802->icount -= CDP1802_CYCLES_INIT;

		if (cdp1802->dmain)
		{
			cdp1802->state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cdp1802->dmaout)
		{
			cdp1802->state = CDP1802_STATE_2_DMA_OUT;
		}
		else
		{
			cdp1802->state = CDP1802_STATE_0_FETCH;
		}

		debugger_instruction_hook(device, cdp1802->r[cdp1802->p]);

		break;

	case CDP1802_STATE_0_FETCH:
		{
		UINT8 opcode = OPCODE_R(R[P]);

		I = opcode >> 4;
		N = opcode & 0x0f;
		R[P] = R[P] + 1;

		cdp1802->icount -= CDP1802_CYCLES_FETCH;

		cdp1802->state = CDP1802_STATE_1_EXECUTE;
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
				cdp1802_short_branch(cdp1802, 1);
				break;

			case 1:
				cdp1802_short_branch(cdp1802, Q == 1);
				break;

			case 2:
				cdp1802_short_branch(cdp1802, D == 0);
				break;

			case 3:
				cdp1802_short_branch(cdp1802, DF == 1);
				break;

			case 4:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF1) ? 0 : 1);
				break;

			case 5:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF2) ? 0 : 1);
				break;

			case 6:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF3) ? 0 : 1);
				break;

			case 7:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF4) ? 0 : 1);
				break;

			case 8:
				cdp1802_short_branch(cdp1802, 0);
				break;

			case 9:
				cdp1802_short_branch(cdp1802, Q == 0);
				break;

			case 0xa:
				cdp1802_short_branch(cdp1802, D != 0);
				break;

			case 0xb:
				cdp1802_short_branch(cdp1802, DF == 0);
				break;

			case 0xc:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF1) ? 1 : 0);
				break;

			case 0xd:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF2) ? 1 : 0);
				break;

			case 0xe:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF3) ? 1 : 0);
				break;

			case 0xf:
				cdp1802_short_branch(cdp1802, (cdp1802->ef & EF4) ? 1 : 0);
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
				cdp1802_add_carry(cdp1802, RAM_R(R[X]), D);
				break;

			case 5:
				cdp1802_sub_carry(cdp1802, RAM_R(R[X]), D);
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
				cdp1802_sub_carry(cdp1802, D, RAM_R(R[X]));
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

				if (cdp1802->intf->q_w)
				{
					cdp1802->intf->q_w(device, Q);
				}
				break;

			case 0xb:
				Q = 1;

				if (cdp1802->intf->q_w)
				{
					cdp1802->intf->q_w(device, Q);
				}
				break;

			case 0xc:
				cdp1802_add_carry(cdp1802, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xd:
				cdp1802_sub_carry(cdp1802, RAM_R(R[P]), D);
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
				cdp1802_sub_carry(cdp1802, D, RAM_R(R[P]));
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
				cdp1802_long_branch(cdp1802, 1);
				break;

			case 1:
				cdp1802_long_branch(cdp1802, Q == 1);
				break;

			case 2:
				cdp1802_long_branch(cdp1802, D == 0);
				break;

			case 3:
				cdp1802_long_branch(cdp1802, DF == 1);
				break;

			case 4:
				// NOP
				break;

			case 5:
				cdp1802_long_skip(cdp1802, Q == 0);
				break;

			case 6:
				cdp1802_long_skip(cdp1802, D != 0);
				break;

			case 7:
				cdp1802_long_skip(cdp1802, DF == 0);
				break;

			case 8:
				cdp1802_long_skip(cdp1802, 1);
				break;

			case 9:
				cdp1802_long_branch(cdp1802, Q == 0);
				break;

			case 0xa:
				cdp1802_long_branch(cdp1802, D != 0);
				break;

			case 0xb:
				cdp1802_long_branch(cdp1802, DF == 0);
				break;

			case 0xc:
				cdp1802_long_skip(cdp1802, IE == 1);
				break;

			case 0xd:
				cdp1802_long_skip(cdp1802, Q == 1);
				break;

			case 0xe:
				cdp1802_long_skip(cdp1802, D == 0);
				break;

			case 0xf:
				cdp1802_long_skip(cdp1802, DF == 1);
				break;
			}

			cdp1802->icount -= CDP1802_CYCLES_EXECUTE;
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
				cdp1802_add(cdp1802, RAM_R(R[X]), D);
				break;

			case 5:
				cdp1802_sub(cdp1802, RAM_R(R[X]), D);
				break;

			case 6:
				DF = D & 0x01;
				D = D >> 1;
				break;

			case 7:
				cdp1802_sub(cdp1802, D, RAM_R(R[X]));
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
				cdp1802_add(cdp1802, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xd:
				cdp1802_sub(cdp1802, RAM_R(R[P]), D);
				R[P] = R[P] + 1;
				break;

			case 0xe:
				DF = (D & 0x80) >> 7;
				D = D << 1;
				break;

			case 0xf:
				cdp1802_sub(cdp1802, D, RAM_R(R[P]));
				R[P] = R[P] + 1;
				break;
			}
			break;
		}

		cdp1802->icount -= CDP1802_CYCLES_EXECUTE;

		if (cdp1802->dmain)
		{
			cdp1802->state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cdp1802->dmaout)
		{
			cdp1802->state = CDP1802_STATE_2_DMA_OUT;
		}
		else if (IE && cdp1802->irq)
		{
			cdp1802->state = CDP1802_STATE_3_INT;
		}
		else if ((I > 0) || (N > 0)) // not idling
		{
			cdp1802->state = CDP1802_STATE_0_FETCH;
		}

		debugger_instruction_hook(device, cdp1802->r[cdp1802->p]);

		break;

    case CDP1802_STATE_2_DMA_IN:

		if (cdp1802->intf->dma_r)
		{
			RAM_W(R[0], cdp1802->intf->dma_r(device, R[0]));
		}

		R[0] = R[0] + 1;

        cdp1802->icount -= CDP1802_CYCLES_DMA;

        if (cdp1802->dmain)
        {
            cdp1802->state = CDP1802_STATE_2_DMA_IN;
        }
        else if (cdp1802->dmaout)
        {
            cdp1802->state = CDP1802_STATE_2_DMA_OUT;
        }
        else if (IE && cdp1802->irq)
        {
            cdp1802->state = CDP1802_STATE_3_INT;
        }
        else if (cdp1802->mode == CDP1802_MODE_LOAD)
        {
            cdp1802->state = CDP1802_STATE_1_EXECUTE;
        }
        else
        {
            cdp1802->state = CDP1802_STATE_0_FETCH;
        }
        break;

    case CDP1802_STATE_2_DMA_OUT:

		if (cdp1802->intf->dma_w)
		{
	        cdp1802->intf->dma_w(device, R[0], RAM_R(R[0]));
		}

		R[0] = R[0] + 1;

        cdp1802->icount -= CDP1802_CYCLES_DMA;

        if (cdp1802->dmain)
        {
            cdp1802->state = CDP1802_STATE_2_DMA_IN;
        }
        else if (cdp1802->dmaout)
        {
            cdp1802->state = CDP1802_STATE_2_DMA_OUT;
        }
        else if (IE && cdp1802->irq)
        {
            cdp1802->state = CDP1802_STATE_3_INT;
        }
        else
        {
            cdp1802->state = CDP1802_STATE_0_FETCH;
        }
        break;

	case CDP1802_STATE_3_INT:

		T = (X << 4) | P;
		X = 2;
		P = 1;
		IE = 0;

		cdp1802->icount -= CDP1802_CYCLES_INTERRUPT;

		if (cdp1802->dmain)
		{
			cdp1802->state = CDP1802_STATE_2_DMA_IN;
		}
		else if (cdp1802->dmaout)
		{
			cdp1802->state = CDP1802_STATE_2_DMA_OUT;
		}
		else
		{
			cdp1802->state = CDP1802_STATE_0_FETCH;
		}

		debugger_instruction_hook(device, cdp1802->r[cdp1802->p]);

		break;
	}
}

static CPU_EXECUTE( cdp1802 )
{
	cdp1802_state *cdp1802 = device->token;

	cdp1802->icount = cycles;

	cdp1802->prevmode = cdp1802->mode;
	cdp1802->mode = cdp1802->intf->mode_r(device);

	do
	{
		switch (cdp1802->mode)
		{
		case CDP1802_MODE_LOAD:
			I = 0;
			N = 0;
			cdp1802->state = CDP1802_STATE_1_EXECUTE;
			cdp1802_run(device);
			break;

		case CDP1802_MODE_RESET:
			cdp1802->state = CDP1802_STATE_1_RESET;
			cdp1802_run(device);
			break;

		case CDP1802_MODE_PAUSE:
			cdp1802->icount -= 1;
			break;

		case CDP1802_MODE_RUN:
			switch (cdp1802->prevmode)
			{
			case CDP1802_MODE_LOAD:
				// RUN mode cannot be initiated from LOAD mode
				cdp1802->mode = CDP1802_MODE_LOAD;
				break;

			case CDP1802_MODE_RESET:
				cdp1802->prevmode = CDP1802_MODE_RUN;
				cdp1802->state = CDP1802_STATE_1_INIT;
				cdp1802_run(device);
				break;

			case CDP1802_MODE_PAUSE:
				cdp1802->prevmode = CDP1802_MODE_RUN;
				cdp1802->state = CDP1802_STATE_0_FETCH;
				cdp1802_run(device);
				break;

			case CDP1802_MODE_RUN:
				cdp1802_run(device);
				break;
			}
			break;
		}
	}
	while (cdp1802->icount > 0);

	return cycles - cdp1802->icount;
}

static CPU_RESET( cdp1802 )
{
	cdp1802_state *cdp1802 = device->token;

	cdp1802->mode = CDP1802_MODE_RESET;
}

static CPU_INIT( cdp1802 )
{
	cdp1802_state *cdp1802 = device->token;

	cdp1802->intf = (cdp1802_interface *) device->static_config;

	/* get address spaces */

	cdp1802->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	cdp1802->io = cpu_get_address_space(device, ADDRESS_SPACE_IO);

	/* set initial values */

	cdp1802->mode = CDP1802_MODE_RESET;
	cdp1802->prevmode = cdp1802->mode;
	cdp1802->irq = CLEAR_LINE;
	cdp1802->dmain = CLEAR_LINE;
	cdp1802->dmaout = CLEAR_LINE;

	/* register for state saving */

	state_save_register_item("cdp1802", device->tag, 0, cdp1802->p);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->x);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->d);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->b);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->t);
	state_save_register_item_array("cdp1802", device->tag, 0, cdp1802->r);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->df);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->ie);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->q);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->n);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->i);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->state);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->prevmode);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->mode);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->irq);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->dmain);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->dmaout);
	state_save_register_item("cdp1802", device->tag, 0, cdp1802->ef);
}

static CPU_GET_CONTEXT( cdp1802 )
{
}

static CPU_SET_CONTEXT( cdp1802 )
{
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( cdp1802 )
{
	cdp1802_state *cdp1802 = device->token;

	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_INT:		cdp1802->irq = info->i;		break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAIN:	cdp1802->dmain = info->i;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAOUT:	cdp1802->dmaout = info->i;	break;

		case CPUINFO_INT_REGISTER + CDP1802_P:			cdp1802->p = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_X:			cdp1802->x = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_T:			cdp1802->t = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_D:			cdp1802->d = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_B:			cdp1802->b = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_R0:			cdp1802->r[0] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R1:			cdp1802->r[1] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R2:			cdp1802->r[2] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R3:			cdp1802->r[3] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R4:			cdp1802->r[4] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R5:			cdp1802->r[5] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R6:			cdp1802->r[6] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R7:			cdp1802->r[7] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R8:			cdp1802->r[8] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_R9:			cdp1802->r[9] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Ra:			cdp1802->r[0xa] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rb:			cdp1802->r[0xb] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rc:			cdp1802->r[0xc] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rd:			cdp1802->r[0xd] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Re:			cdp1802->r[0xe] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_Rf:			cdp1802->r[0xf] = info->i;	break;
		case CPUINFO_INT_REGISTER + CDP1802_DF:			cdp1802->df = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_IE:			cdp1802->ie = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_Q:			cdp1802->q = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_N:			cdp1802->n = info->i;		break;
		case CPUINFO_INT_REGISTER + CDP1802_I:			cdp1802->i = info->i;		break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CDP1802_PC: 		cdp1802->r[cdp1802->p] = info->i;	break;
	}
}

/****************************************************************************
 * Return a formatted string for a register
 ****************************************************************************/
CPU_GET_INFO( cdp1802 )
{
	cdp1802_state *cdp1802 = (device != NULL) ? device->token : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cdp1802_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
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

		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_INT:		info->i = cdp1802->irq;		break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAIN:	info->i = cdp1802->dmain;	break;
		case CPUINFO_INT_INPUT_STATE + CDP1802_INPUT_LINE_DMAOUT:	info->i = cdp1802->dmaout;	break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_SP:							info->i = 0;							break;
		case CPUINFO_INT_REGISTER + CDP1802_P:			info->i = cdp1802->p;					break;
		case CPUINFO_INT_REGISTER + CDP1802_X:			info->i = cdp1802->x;					break;
		case CPUINFO_INT_REGISTER + CDP1802_T:			info->i = cdp1802->t;					break;
		case CPUINFO_INT_REGISTER + CDP1802_D:			info->i = cdp1802->d;					break;
		case CPUINFO_INT_REGISTER + CDP1802_B:			info->i = cdp1802->b;					break;
		case CPUINFO_INT_REGISTER + CDP1802_R0:			info->i = cdp1802->r[0];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R1:			info->i = cdp1802->r[1];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R2:			info->i = cdp1802->r[2];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R3:			info->i = cdp1802->r[3];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R4:			info->i = cdp1802->r[4];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R5:			info->i = cdp1802->r[5];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R6:			info->i = cdp1802->r[6];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R7:			info->i = cdp1802->r[7];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R8:			info->i = cdp1802->r[8];				break;
		case CPUINFO_INT_REGISTER + CDP1802_R9:			info->i = cdp1802->r[9];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Ra:			info->i = cdp1802->r[0xa];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rb:			info->i = cdp1802->r[0xb];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rc:			info->i = cdp1802->r[0xc];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rd:			info->i = cdp1802->r[0xd];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Re:			info->i = cdp1802->r[0xe];				break;
		case CPUINFO_INT_REGISTER + CDP1802_Rf:			info->i = cdp1802->r[0xf];				break;
		case CPUINFO_INT_REGISTER + CDP1802_DF:			info->i = cdp1802->df;					break;
		case CPUINFO_INT_REGISTER + CDP1802_IE:			info->i = cdp1802->ie;					break;
		case CPUINFO_INT_REGISTER + CDP1802_Q:			info->i = cdp1802->q;					break;
		case CPUINFO_INT_REGISTER + CDP1802_N:			info->i = cdp1802->n;					break;
		case CPUINFO_INT_REGISTER + CDP1802_I:			info->i = cdp1802->i;					break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CDP1802_PC:			info->i = cdp1802->r[cdp1802->p];		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cdp1802);			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(cdp1802);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(cdp1802);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(cdp1802);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(cdp1802);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cdp1802);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;									break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cdp1802);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cdp1802->icount;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "CDP1802");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "RCA COSMAC");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

		case CPUINFO_STR_REGISTER + CDP1802_PC:	sprintf(info->s, "PC:%.4x", cdp1802->r[cdp1802->p]);break;
		case CPUINFO_STR_REGISTER + CDP1802_R0:	sprintf(info->s, "R0:%.4x", cdp1802->r[0]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R1:	sprintf(info->s, "R1:%.4x", cdp1802->r[1]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R2:	sprintf(info->s, "R2:%.4x", cdp1802->r[2]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R3:	sprintf(info->s, "R3:%.4x", cdp1802->r[3]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R4:	sprintf(info->s, "R4:%.4x", cdp1802->r[4]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R5:	sprintf(info->s, "R5:%.4x", cdp1802->r[5]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R6:	sprintf(info->s, "R6:%.4x", cdp1802->r[6]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R7:	sprintf(info->s, "R7:%.4x", cdp1802->r[7]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R8:	sprintf(info->s, "R8:%.4x", cdp1802->r[8]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_R9:	sprintf(info->s, "R9:%.4x", cdp1802->r[9]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Ra:	sprintf(info->s, "Ra:%.4x", cdp1802->r[0xa]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Rb:	sprintf(info->s, "Rb:%.4x", cdp1802->r[0xb]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Rc:	sprintf(info->s, "Rc:%.4x", cdp1802->r[0xc]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Rd:	sprintf(info->s, "Rd:%.4x", cdp1802->r[0xd]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Re:	sprintf(info->s, "Re:%.4x", cdp1802->r[0xe]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_Rf:	sprintf(info->s, "Rf:%.4x", cdp1802->r[0xf]); 		break;
		case CPUINFO_STR_REGISTER + CDP1802_P:	sprintf(info->s, "P:%x",    cdp1802->p);			break;
		case CPUINFO_STR_REGISTER + CDP1802_X:	sprintf(info->s, "X:%x",    cdp1802->x);			break;
		case CPUINFO_STR_REGISTER + CDP1802_D:	sprintf(info->s, "D:%.2x",  cdp1802->d);			break;
		case CPUINFO_STR_REGISTER + CDP1802_B:	sprintf(info->s, "B:%.2x",  cdp1802->b);			break;
		case CPUINFO_STR_REGISTER + CDP1802_T:	sprintf(info->s, "T:%.2x",  cdp1802->t);			break;
		case CPUINFO_STR_REGISTER + CDP1802_DF:	sprintf(info->s, "DF:%x",   cdp1802->df);			break;
		case CPUINFO_STR_REGISTER + CDP1802_IE:	sprintf(info->s, "IE:%x",   cdp1802->ie);			break;
		case CPUINFO_STR_REGISTER + CDP1802_Q:	sprintf(info->s, "Q:%x",    cdp1802->q);			break;
		case CPUINFO_STR_REGISTER + CDP1802_N:	sprintf(info->s, "N:%x",    cdp1802->n);			break;
		case CPUINFO_STR_REGISTER + CDP1802_I:	sprintf(info->s, "I:%x",    cdp1802->i);			break;
		case CPUINFO_STR_FLAGS: sprintf(info->s,
									"%s%s%s",
									 cdp1802->df ? "DF" : "..",
									 cdp1802->ie ? "IE" : "..",
									 cdp1802->q ? "Q" : "."); break;
	}
}
