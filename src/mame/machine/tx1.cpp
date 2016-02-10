// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi TX-1/Buggy Boy machine hardware

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cpu/i86/i86.h"
#include "includes/tx1.h"


/*
    Helper functions
*/
#define INC_PROM_ADDR       ( math.promaddr = (math.promaddr + 1) & 0x1ff )
#define ROR16(val, shift)   ( ((UINT16)val >> shift) | ((UINT16)val << (16 - shift)) )
#define ROL16(val, shift)   ( ((UINT16)val << shift) | ((UINT16)val >> (16 - shift)) )
#define SWAP16(val)         ( (((UINT16)val << 8) & 0xff00) | ((UINT16)val >> 8) )

static inline UINT8 reverse_nibble(UINT8 nibble)
{
	return  (nibble & 1) << 3 |
			(nibble & 2) << 1 |
			(nibble & 4) >> 1 |
			(nibble & 8) >> 3;
}


/*
    State transition table

    A little different to the real thing in that
    there are no states between final input and
    multiplication/division.
*/
static const UINT8 state_table[16][8] =
{
	{  4,  4,  4,  4,  5,  1,  1,  0 },
	{  4,  4,  4,  4,  5,  5,  3,  0 },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{  4,  4,  4,  4,  5,  5, 11,  0 },
	{  8,  8,  8,  8,  8,  8,  8,  8 },
	{ 10, 10, 10, 10, 10, 10, 10, 10 },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{  4,  4,  4,  4,  5,  0,  1,  0 },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{  4,  4,  4,  4,  4,  5,  1,  0 },
	{  4,  4,  4,  4,  5,  5,  1,  0 },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
	{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf },
};

static void sn_multiply(running_machine &machine)
{
	tx1_state *state = machine.driver_data<tx1_state>();
	sn74s516_t &SN74S516 = state->m_sn74s516;

	switch (SN74S516.code)
	{
		case 0:
		{
			SN74S516.ZW.ZW32 = SN74S516.X * SN74S516.Y;
			break;
		}
		case 2:
		{
			SN74S516.ZW.ZW32 += SN74S516.X * SN74S516.Y;
			break;
		}
		case 3:
		{
			SN74S516.ZW.ZW32 += -SN74S516.X * SN74S516.Y;
			break;
		}
		case 0x60:
		{
			SN74S516.ZW.ZW32 = SN74S516.X * SN74S516.Y;
			break;
		}
		case 0x61:
		{
			SN74S516.ZW.ZW32 = -SN74S516.X * SN74S516.Y;
			break;
		}
		case 0x62:
		{
			SN74S516.ZW.ZW32 += SN74S516.X * SN74S516.Y;
			break;
		}
		case 0x63:
		{
			SN74S516.ZW.ZW32 += -SN74S516.X * SN74S516.Y;
			break;
		}
		case 0x660:
		{
			SN74S516.ZW.ZW32 = (SN74S516.X * SN74S516.Y) + (SN74S516.ZW.ZW32 & 0xffff0000);
			break;
		}
		case 0x661:
		{
			SN74S516.ZW.ZW32 = (-SN74S516.X * SN74S516.Y) + (SN74S516.ZW.ZW32 & 0xffff0000);
			break;
		}
		case 0x662:
		{
			SN74S516.ZW.ZW32 = (-SN74S516.X * SN74S516.Y) + (SN74S516.ZW.ZW32 & 0xffff0000);
			break;
		}
		case 0x6660:
		{
			SN74S516.ZW.ZW32 += (SN74S516.X * SN74S516.Y);
			break;
		}
		default:
		{
			osd_printf_debug("sn74s516 ??? multiply: %x\n", SN74S516.code);
		}
	}

	/* Seems a good enough place to clear it. */
	SN74S516.ZWfl = 0;
}

static void sn_divide(running_machine &machine)
{
	tx1_state *state = machine.driver_data<tx1_state>();
	sn74s516_t &SN74S516 = state->m_sn74s516;
	INT32 Z = 0;
	INT32 W = 0;

	if (SN74S516.X == 0)
	{
		osd_printf_debug("%s:SN74S516 tried to divide by zero\n", machine.describe_context());
		SN74S516.ZW.as16bit.Z = (INT16)0xffff;
		SN74S516.ZW.as16bit.W = 0xffff;
		SN74S516.ZWfl = 0;
		return;
	}

	switch (SN74S516.code)
	{
		case 4:
		{
			Z = SN74S516.ZW.ZW32 / SN74S516.X;
			W = SN74S516.ZW.ZW32 % SN74S516.X;
			break;
		}
		case 0x664:
		{
			Z = SN74S516.ZW.ZW32 / SN74S516.X;
			W = SN74S516.ZW.ZW32 % SN74S516.X;
			break;
		}
		case 0x6664:
		{
			Z = SN74S516.ZW.as16bit.W / SN74S516.X;
			W = SN74S516.ZW.as16bit.W % SN74S516.X;
			break;
		}
		default:
		{
			osd_printf_debug("SN74S516 unhandled divide type: %x\n", SN74S516.code);
		}
	}

	/* Divide overflow Only happens during chip test anyway */
	if (Z > 0xffff)
		Z |= 0xff00;

	SN74S516.ZW.as16bit.Z = Z;
	SN74S516.ZW.as16bit.W = W;
	SN74S516.ZWfl = 0;
}

static void sn74s516_update(running_machine &machine, int ins)
{
	tx1_state *state = machine.driver_data<tx1_state>();
	sn74s516_t &SN74S516 = state->m_sn74s516;
	SN74S516.state = state_table[SN74S516.state][ins];

	if (SN74S516.state == 4)
	{
		sn_multiply(machine);
		SN74S516.state = 8;
	}
	else if (SN74S516.state == 5)
	{
		sn_divide(machine);
		SN74S516.state = 10;
	}
}

static void kick_sn74s516(running_machine &machine, UINT16 *data, const int ins)
{
	tx1_state *state = machine.driver_data<tx1_state>();
	sn74s516_t &SN74S516 = state->m_sn74s516;
	math_t &math = state->m_math;

#define LOAD_X      (SN74S516.X = *data)
#define LOAD_Y      (SN74S516.Y = *data)
#define LOAD_Z      (SN74S516.ZW.as16bit.Z = *data)
#define LOAD_W      (SN74S516.ZW.as16bit.W = *data)
#define READ_ZW     *data = SN74S516.ZWfl ? SN74S516.ZW.as16bit.W : SN74S516.ZW.as16bit.Z; \
					SN74S516.ZWfl ^= 1;

#define UPDATE_SEQUENCE (SN74S516.code = (SN74S516.code << 4) | ins)
#define CLEAR_SEQUENCE  (SN74S516.code = 0)

	/*
	    Remember to change the Z/W flag.
	*/
	switch (SN74S516.state)
	{
		case 0:
		{
			CLEAR_SEQUENCE;
			UPDATE_SEQUENCE;

			if (ins < 4)
			{
				LOAD_Y;
				sn74s516_update(machine, ins);
			}
			else if (ins == 4)
			{
				sn74s516_update(machine, ins);
			}
			else if (ins < 7)
			{
				LOAD_X;
				sn74s516_update(machine, ins);
			}
			else if (ins == 7)
			{
				READ_ZW;
				break;
			}

			break;
		}
		case 8:
		case 10:
		{
			CLEAR_SEQUENCE;
			UPDATE_SEQUENCE;

			if (ins < 4)
			{
				LOAD_Y;
				sn74s516_update(machine, ins);
			}
			else if (ins == 4)
			{
				sn74s516_update(machine, ins);
			}
			else if (ins == 5)
			{
				// Rounding
				// Operation
				sn74s516_update(machine, ins);
			}
			else if (ins == 6)
			{
				LOAD_X;
				sn74s516_update(machine, ins);
			}
			else if (ins == 7)
			{
				READ_ZW;
				sn74s516_update(machine, ins);
			}
			break;
		}
		case 1:
		{
			// TODO: 6666 represents an incomplete state - clear it.
			if (SN74S516.code == 0x6666)
			{
				CLEAR_SEQUENCE;
				osd_printf_debug("%s:Code 6666: PROMADDR:%x\n", machine.describe_context(), math.promaddr);
			}

			UPDATE_SEQUENCE;
			if (ins < 4)
			{
				LOAD_Y;
				sn74s516_update(machine, ins);
			}
			else if (ins < 6)
			{
				sn74s516_update(machine, ins);
			}
			else if (ins == 6)
			{
				LOAD_Z;
				sn74s516_update(machine, ins);
			}
			else if (ins == 7)
			{
				// Pointless operation.
				sn74s516_update(machine, ins);
			}

			break;
		}
		case 3:
		{
			UPDATE_SEQUENCE;
			if (ins < 4)
			{
				LOAD_Y;
				sn74s516_update(machine, ins);
			}
			else if (ins == 4)
			{
				LOAD_W;
				sn74s516_update(machine, ins);
			}
			else if (ins == 5)
			{
				sn74s516_update(machine, ins);
			}
			else if (ins == 6)
			{
				LOAD_W;
				sn74s516_update(machine, ins);
			}
			else if (ins == 7)
			{
				READ_ZW;
				sn74s516_update(machine, ins);
			}
			break;
		}
		case 11:
		{
			UPDATE_SEQUENCE;
			if (ins < 4)
			{
				LOAD_Y;
				sn74s516_update(machine, ins);
			}
			else if (ins < 6)
			{
				sn74s516_update(machine, ins);
			}
			else if (ins == 6)
			{
				// CHECK: Incomplete state
				sn74s516_update(machine, ins);
			}
			else if (ins == 7)
			{
				/* 6667 = Load X, Load Z, Load W, Clear Z */
				SN74S516.ZW.as16bit.Z = 0;
				sn74s516_update(machine, ins);
			}
			break;
		}
		default:
		{
			osd_printf_debug("Unknown SN74S516 state. %x\n", SN74S516.code);
		}
	}

	math.dbgaddr = math.promaddr;
	math.dbgpc = machine.device("math_cpu")->safe_pcbase();
}


/***************************************************************************

  TX-1

  Preliminary

***************************************************************************/

/* Same mapping as Buggy Boy actually */
#define TX1_INSLD       0x100
#define TX1_CNTST       0x80
#define TX1_RADCHG      0x20
#define TX1_DSEL        0x03

enum
{
	TX1_SEL_MULEN = 0x00,
	TX1_SEL_PPSEN,
	TX1_SEL_PSSEN,
	TX1_SEL_LMSEL,
	TX1_SEL_DSELOE,
	TX1_SEL_INSCL = 0x06,
	TX1_SEL_ILDEN
};

#define TX1_SET_INS0_BIT    do { if (!(ins & 0x4) && math.i0ff) ins |= math.i0ff; } while(0)

static inline UINT16 get_tx1_datarom_addr(math_t &math)
{
	UINT16 addr;

	addr = ((math.inslatch & 0x1c00) << 1) | (math.ppshift & 0xff);

	if ((math.inslatch >> 8) & TX1_RADCHG)
		addr |= (math.ppshift & 0x0700);
	else
		addr |= (math.promaddr << 3) & 0x0700;

	return addr & 0x3fff;
}

static void tx1_update_state(running_machine &machine)
{
#define LHIEN(a)    !(a & 0x80)
#define LLOEN(a)    !(a & 0x40)
#define GO_EN(a)    !(a & 0x4000)

	tx1_state *state = machine.driver_data<tx1_state>();
	math_t &math = state->m_math;
	const UINT16 *prom = (UINT16*)state->memregion("au_data")->base() + (0x8000 >> 1);

	for (;;)
	{
		int go = 0;

		if (!GO_EN(math.inslatch) && GO_EN(prom[math.promaddr]))
			go = 1;
		/*
		    Example:
		    120 /GO /LHIEN
		    121 /GO        /LLOEN
		    Both 120 and 121 are used.
		*/
		else if ((GO_EN(math.inslatch) && GO_EN(prom[math.promaddr])) && (LHIEN(math.inslatch) && LLOEN(prom[math.promaddr])))
			go = 1;

		/* Now update the latch */
		math.inslatch = prom[math.promaddr] & 0x7fff;
		math.mux = (math.inslatch >> 3) & 7;

		if (math.mux == TX1_SEL_INSCL)
		{
			math.i0ff = 0;
		}
		else if (math.mux == TX1_SEL_PPSEN)
		{
			// NOTE: Doesn't do anything without SPCS.
		}

		/* TODO */
		if (go)
		{
			int ins = math.inslatch & 7;

			TX1_SET_INS0_BIT;

			if (math.mux == TX1_SEL_DSELOE)
			{
				int     dsel = (math.inslatch >> 8) & TX1_DSEL;
				int     tfad = (math.inslatch & 0x1c00) << 1;
				int     sd   = math.ppshift;
				int     o4;
				UINT16  data;

				o4 =
					(!BIT(sd, 9) && !BIT(sd,10)) ||
					( BIT(sd, 7) &&  BIT(sd,10)) ||
					(!BIT(sd, 8) &&  BIT(sd, 9)) ||
					(!BIT(sd, 7) &&  BIT(sd, 8)) ||
					!BIT(dsel, 1) || BIT(tfad, 13) || BIT(tfad, 12) || BIT(tfad, 11);

				dsel = (dsel & 2) | ((dsel & o4) ^ 1);

				if (dsel == 0)
					data = math.muxlatch;
				else if (dsel == 1)
				{
					UINT16 *romdata = (UINT16*)machine.root_device().memregion("au_data")->base();
					UINT16 addr = get_tx1_datarom_addr(math);
					data = romdata[addr];
				}
				else if (dsel == 2)
					data = ROL16(math.muxlatch, 4);
				else if (dsel == 3)
					data = ROL16(SWAP16(math.muxlatch), 3);

				kick_sn74s516(machine, &data, ins);
			}
			/*
			    TODO: Changed ppshift to muxlatch for TX-1

			    /TMPLD1: /LHIEN
			    /TMPLD2: /LLOEN.!O4 + (/LHIEN.O4)
			    /TMPLD3: /LLOEN
			         O4: !SD9.!SD10./LMSEL + SD7.SD10./LMSEL +
			             !SD8.SD9./LMSEL + !SD7.SD8./LMSEL +
			             /LMSEL./DSEL1 + /LMSEL.TFAD13 + /LMSEL.TFAD12 + /LMSEL.TFAD11
			*/
			else if (LHIEN(math.inslatch) || LLOEN(math.inslatch))
			{
				UINT16 data;

				kick_sn74s516(machine, &data, ins);

				/* All latches enabled */
				if (LHIEN(math.inslatch) && LLOEN(math.inslatch))
				{
					math.muxlatch = data;
				}
				else if (math.mux == TX1_SEL_LMSEL) // O4 = 0
				{
					// TMPLD2/TMPLD3 15-5
					if (LLOEN(math.inslatch))
					{
						math.muxlatch &= 0x001f;
						math.muxlatch |= data & 0xffe0;
					}
					// TMLPD1 4-0???????
					else if (LHIEN(math.inslatch))
					{
						math.muxlatch &= 0xffe0;
						math.muxlatch |= data & 0x001f;
					}
				}
				else
				{
					/*
					    /TMPLD1: /LHIEN
					    /TMPLD2: /LLOEN.!O4 + /LHIEN.O4
					    /TMPLD3: /LLOEN
					     O4: !SD9.!SD10./LMSEL + SD7.SD10./LMSEL +
					         !SD8.SD9./LMSEL + !SD7.SD8./LMSEL +
					         /LMSEL./DSEL1 + /LMSEL.TFAD13 + /LMSEL.TFAD12 + /LMSEL.TFAD11
					*/
					int     dsel = (math.inslatch >> 8) & TX1_DSEL;
					int     tfad = (math.inslatch & 0x1c00) << 1;
					int     sd   = math.ppshift;
					int     o4;

					o4 =
						(!BIT(sd, 9) && !BIT(sd,10)) ||
						( BIT(sd, 7) &&  BIT(sd,10)) ||
						(!BIT(sd, 8) &&  BIT(sd, 9)) ||
						(!BIT(sd, 7) &&  BIT(sd, 8)) ||
						!BIT(dsel, 1) || BIT(tfad, 13) || BIT(tfad, 12) || BIT(tfad, 11);

					if (LLOEN(math.inslatch))
					{
						math.muxlatch &= 0x0fff;
						math.muxlatch |= data & 0xf000;

						if (!o4)
						{
							// TMPLD11-5
							math.muxlatch &= 0xf01f;
							math.muxlatch |= data & 0x0fe0;
						}
					}
					else if (LHIEN(math.inslatch))
					{
						math.muxlatch &= 0xffe0;
						math.muxlatch |= data & 0x001f;

						if (o4)
						{
							// TMPLD11-5
							math.muxlatch &= 0xf01f;
							math.muxlatch |= data & 0x0fe0;
						}
					}
				}
			}
			else
			{
				if (math.mux == TX1_SEL_PPSEN)
				{
					kick_sn74s516(machine, &math.ppshift, ins);
				}
				else
				{
					/* Bus pullups give 0xffff */
					UINT16 data = 0xffff;
					kick_sn74s516(machine, &data, ins);
				}
			}
		}

		/* Is there another instruction in the sequence? */
		if (prom[math.promaddr] & 0x8000)
			break;
		else
			INC_PROM_ADDR;
	}
}

READ16_MEMBER(tx1_state::tx1_math_r)
{
	math_t &math = m_math;
	offset = offset << 1;

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = math.inslatch & 7;
			TX1_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		/* TODO What do we return? */
		kick_sn74s516(machine(), &math.retval, ins);
	}
	/* /PPSEN */
	else if (offset < 0x800)
	{
		// Unused - just pullups?
		math.retval = 0xffff;
	}
	/* /MUXCS */
	else if ((offset & 0xc00) == 0xc00)
	{
		int     dsel = (math.inslatch >> 8) & TX1_DSEL;
		int     tfad = (math.inslatch & 0x1c00) << 1;
		int     sd   = math.ppshift;
		int     o4;

		if (math.mux == TX1_SEL_LMSEL)
			o4 = 0;
		else
		{
			o4 =
			(!BIT(sd, 9) && !BIT(sd,10)) ||
			( BIT(sd, 7) &&  BIT(sd,10)) ||
			(!BIT(sd, 8) &&  BIT(sd, 9)) ||
			(!BIT(sd, 7) &&  BIT(sd, 8)) ||
			!BIT(dsel, 1) || BIT(tfad, 13) || BIT(tfad, 12) || BIT(tfad, 11);
		}

		dsel = (dsel & 2) | ((dsel & o4) ^ 1);

		if (dsel == 0)
			math.retval = math.muxlatch;
		else if (dsel == 1 )
		{
			/*
			    TODO make this constant somewhere
			    e.g. math.retval =  math.romptr[ get_tx1_datarom_addr() ];
			*/
			UINT16 *romdata = (UINT16*)memregion("au_data")->base();
			UINT16 addr = get_tx1_datarom_addr(math);
			math.retval = romdata[addr];
		}
		else if (dsel == 2)
			math.retval = ROL16(math.muxlatch, 4);
		else if (dsel == 3)
			math.retval = ROL16(SWAP16(math.muxlatch), 3);

		/* TODO for TX-1: This is /SPCS region? */
		if (offset < 0xe00)
		{
			// Load the PP with retval??????
			if (math.mux == TX1_SEL_PPSEN)
			{
				math.ppshift = math.retval & 0x3fff;
			}
			else if (math.mux == TX1_SEL_PSSEN)
			{
				// WRONG!!!!
				osd_printf_debug("Math Read with PSSEN!\n");
				math.ppshift = math.retval;
			}

			if (math.mux != TX1_SEL_ILDEN)
			{
				INC_PROM_ADDR;
				tx1_update_state(machine());

				// MUST RETURN HERE?
				return math.retval;
			}
		}
	}
	else
	{
		if (math.mux == TX1_SEL_PPSEN)
			math.retval = math.ppshift & 0x3fff;
		else
			/* Nothing is mapped - read from pull up resistors! */
			math.retval = 0xffff;
	}

	if (offset & TX1_INSLD)
	{
		math.promaddr = (offset << 2) & 0x1ff;
		tx1_update_state(machine());
	}
	else if (offset & TX1_CNTST)
	{
		INC_PROM_ADDR;
		tx1_update_state(machine());
	}

	return math.retval;
}

WRITE16_MEMBER(tx1_state::tx1_math_w)
{
	math_t &math = m_math;
	math.cpulatch = data;
	offset <<= 1;

//  printf("W %x: %x\n", 0x3000 + offset, data);

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = math.inslatch & 7;
			TX1_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		kick_sn74s516(machine(), &math.cpulatch, ins);
	}
	/* /PPSEN */
	else if ((offset & 0xc00) == 0x400)
	{
		/* Input is 14 bits */
		math.ppshift = math.cpulatch & 0x3fff;
	}
	/* /PSSEN */
	else if ((offset & 0xc00) == 0x800)
	{
		//if (((math.inslatch >> 8) & TX1_DSEL) == 3 )
		{
			int shift;
			UINT16 val = math.ppshift;

			if (math.cpulatch & 0x3800)
			{
				shift = (math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val >>= 1;
					shift >>= 1;
				}
			}
			else
			{
				shift = (math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val <<= 1;
					shift >>= 1;
				}
			}
			math.ppshift = val;
		}
	}
	/* /MUXCS */
	else if ((offset & 0xc00) == 0xc00)
	{
		/*
		    /TMPLD1: 0
		    /TMPLD2: 0
		    /TMPLD3: 0
		*/
		math.muxlatch = math.cpulatch;
	}

	if (offset & TX1_INSLD)
	{
		math.promaddr = (offset << 2) & 0x1ff;
		tx1_update_state(machine());
	}
	else if (offset & TX1_CNTST)
	{
		INC_PROM_ADDR;
		tx1_update_state(machine());
	}
}

READ16_MEMBER(tx1_state::tx1_spcs_rom_r)
{
	math_t &math = m_math;
	math.cpulatch = *(UINT16*)((UINT8*)memregion("math_cpu")->base() + 0x04000 + 0x1000 + offset*2);

	if (math.mux == TX1_SEL_ILDEN)
	{
		math.i0ff = math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (math.mux == TX1_SEL_MULEN)
	{
		int ins = math.inslatch & 7;

		TX1_SET_INS0_BIT;
		kick_sn74s516(machine(), &math.cpulatch, ins);
	}
	else if (math.mux == TX1_SEL_PPSEN)
	{
		math.ppshift = math.cpulatch;
	}
	else if (math.mux == TX1_SEL_PSSEN)
	{
			//if ( ((math.inslatch >> 8) & TX1_DSEL) == 3 )
		{
			int shift;
			UINT16 val = math.ppshift;

			if (math.cpulatch & 0x3800)
			{
				shift = (math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val >>= 1;
					shift >>= 1;
				}
			}
			else
			{
				shift = (math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val <<= 1;
					shift >>= 1;
				}
			}
			math.ppshift = val & 0x7ff;
		}
	}

	if (math.mux != TX1_SEL_ILDEN)
	{
		INC_PROM_ADDR;
		tx1_update_state(machine());
	}

	return math.cpulatch;

}

READ16_MEMBER(tx1_state::tx1_spcs_ram_r)
{
	math_t &math = m_math;
	math.cpulatch = m_math_ram[offset];

	offset <<= 1;

	if (math.mux == TX1_SEL_ILDEN)
	{
		math.i0ff = math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (math.mux == TX1_SEL_MULEN)
	{
		int ins = math.inslatch & 7;

		TX1_SET_INS0_BIT;
		kick_sn74s516(machine(), &math.cpulatch, ins);
	}
	else if (math.mux == TX1_SEL_PPSEN)
	{
//      math.ppshift = math.retval & 0x3fff;
		math.ppshift = math.cpulatch;
	}
	else if (math.mux == TX1_SEL_PSSEN)
	{
		int shift;
		UINT16 val = math.ppshift;

		if (math.cpulatch & 0x3800)
		{
			shift = (math.cpulatch >> 11) & 0x7;

			while (shift)
			{
				val >>= 1;
				shift >>= 1;
			}
		}
		else
		{
			shift = (math.cpulatch >> 7) & 0xf;
			shift = reverse_nibble(shift);
			shift >>= 1;

			while (shift)
			{
				val <<= 1;
				shift >>= 1;
			}
		}
		math.ppshift = val & 0x7ff;
	}

	if (math.mux != TX1_SEL_ILDEN)
	{
		INC_PROM_ADDR;
		tx1_update_state(machine());
	}

	return math.cpulatch;
}

/* Should never occur */
WRITE16_MEMBER(tx1_state::tx1_spcs_ram_w)
{
	osd_printf_debug("Write to /SPCS RAM?");
	COMBINE_DATA(&m_math_ram[offset]);
}


/***************************************************************************

  Buggy Boy

***************************************************************************/
#define BB_INSLD        0x100
#define BB_CNTST        0x80
#define BB_RADCHG       0x20
#define BB_DSEL         0x03

enum
{
	BB_MUX_MULEN = 0x00,
	BB_MUX_PPSEN,
	BB_MUX_PSSEN,
	BB_MUX_LMSEL,
	BB_MUX_DPROE,
	BB_MUX_PPOE,
	BB_MUX_INSCL,
	BB_MUX_ILDEN
};

#define BB_SET_INS0_BIT do { if (!(ins & 0x4) && math.i0ff) ins |= math.i0ff;} while(0)

static inline UINT16 get_bb_datarom_addr(math_t &math)
{
	UINT16 addr;

	addr = ((math.inslatch & 0x1c00) << 1) | (math.ppshift & 0xff);

	if ((math.inslatch >> 8) & BB_RADCHG)
	{
		addr |= (math.ppshift & 0x0700);
	}
	else
	{
		addr |= (math.promaddr << 3) & 0x0700;
	}

	return addr & 0x3fff;
}

static void buggyboy_update_state(running_machine &machine)
{
#define LHIEN(a)    !(a & 0x80)
#define LLOEN(a)    !(a & 0x40)
#define GO_EN(a)    !(a & 0x4000)

	tx1_state *state = machine.driver_data<tx1_state>();
	math_t &math = state->m_math;
	const UINT16 *prom = (UINT16*)state->memregion("au_data")->base() + (0x8000 >> 1);

	for (;;)
	{
		int go = 0;

		if (!GO_EN(math.inslatch) && GO_EN(prom[math.promaddr]))
			go = 1;
		else if ((GO_EN(math.inslatch) && GO_EN(prom[math.promaddr])) && (LHIEN(math.inslatch) && LLOEN(prom[math.promaddr])))
			go = 1;

		/* Now update the latch */
		math.inslatch = prom[math.promaddr] & 0x7fff;
		math.mux = (math.inslatch >> 3) & 7;

		if (math.mux == BB_MUX_INSCL)
			math.i0ff = 0;
		else if (math.mux == BB_MUX_PPSEN)
		{
			// TODO: Needed?
			//osd_printf_debug("/PPSEN with INS: %x\n", math.promaddr);
			//math.ppshift = lastval;//math.cpulatch;
		}

		/* TODO */
		if (go)
		{
			int ins = math.inslatch & 7;

			BB_SET_INS0_BIT;

			if (math.mux == BB_MUX_DPROE)
			{
				UINT16 *romdata = (UINT16*)machine.root_device().memregion("au_data")->base();
				UINT16 addr = get_bb_datarom_addr(math);
				kick_sn74s516(machine, &romdata[addr], ins);
			}
			else if (math.mux == BB_MUX_PPOE)
			{
				kick_sn74s516(machine, &math.ppshift, ins);
			}
			/* This is quite tricky. */
			/* It can either be a read operation or */
			/* What if /LHIEN and /LLOEN? */
			else if (LHIEN(math.inslatch) || LLOEN(math.inslatch))
			{
				UINT16 data;

				kick_sn74s516(machine, &data, ins);

				if (LHIEN(math.inslatch) && LLOEN(math.inslatch))
				{
					math.ppshift = data;
				}
				else if (math.mux == BB_MUX_LMSEL)
				{
					if (LLOEN(math.inslatch))
					{
						math.ppshift &= 0x000f;
						math.ppshift |= data & 0xfff0;
					}
					else if (LHIEN(math.inslatch))
					{
						math.ppshift &= 0xfff0;
						math.ppshift |= data & 0x000f;
					}
				}
				else
				{
					if (LLOEN(math.inslatch))
					{
						math.ppshift &= 0x0fff;
						math.ppshift |= data & 0xf000;
					}
					else if (LHIEN(math.inslatch))
					{
						math.ppshift &= 0xf000;
						math.ppshift |= data & 0x0fff;
					}
				}
			}
			else
			{
				if (math.mux == BB_MUX_PPSEN)
				{
					kick_sn74s516(machine, &math.ppshift, ins);
				}
				else
				{
					/* Bus pullups give 0xffff */
					UINT16 data = 0xffff;
					kick_sn74s516(machine, &data, ins);
				}
			}
		}

		/* Handle rotation */
		if (((math.inslatch >> 8) & BB_DSEL) == 1)
		{
			math.ppshift = ROR16(math.ppshift, 4);
		}
		else if (((math.inslatch >> 8) & BB_DSEL) == 2)
		{
			math.ppshift = ROL16(math.ppshift, 4);
		}

		/* Is there another instruction in the sequence? */
		if (prom[math.promaddr] & 0x8000)
			break;
		else
			INC_PROM_ADDR;
	}
}

READ16_MEMBER(tx1_state::buggyboy_math_r)
{
	math_t &math = m_math;
	offset = offset << 1;

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = math.inslatch & 7;
			BB_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		/* TODO What do we return? */
		kick_sn74s516(machine(), &math.retval, ins);

		/* TODO */
		//if (math.mux == BB_MUX_PPSEN)
		//  math.ppshift = math.retval;
	}
	/* /PPSEN */
	else if (offset < 0x800)
	{
		math.retval = math.ppshift;
	}
	/* /DPROE */
	else if ((offset & 0xc00) == 0xc00)
	{
		UINT16 *romdata = (UINT16*)memregion("au_data")->base();
		UINT16 addr = get_bb_datarom_addr(math);

		math.retval = romdata[addr];

		/* This is necessary */
		if (math.mux == BB_MUX_PPSEN)
			math.ppshift = romdata[addr];

		/* This is /SPCS region? Necessary anyway */
		if (offset < 0xe00)
		{
			if (math.mux != BB_MUX_ILDEN)
			{
				INC_PROM_ADDR;
				buggyboy_update_state(machine());
			}
		}
	}
	else
	{
		if (math.mux == BB_MUX_PPSEN)
			math.retval = math.ppshift;
		else
			/* Nothing is mapped - read from pull up resistors! */
			math.retval = 0xffff;
	}

	if (offset & BB_INSLD)
	{
		math.promaddr = (offset << 2) & 0x1ff;
		buggyboy_update_state(machine());
	}
	else if (offset & BB_CNTST)
	{
		INC_PROM_ADDR;
		buggyboy_update_state(machine());
	}

	return math.retval;
}

WRITE16_MEMBER(tx1_state::buggyboy_math_w)
{
	math_t &math = m_math;
	math.cpulatch = data;

	offset <<= 1;

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = math.inslatch & 7;
			BB_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		kick_sn74s516(machine(), &math.cpulatch, ins);
	}
	/* /PPSEN */
	else if ((offset & 0xc00) == 0x400)
	{
		math.ppshift = math.cpulatch;
	}
	/* /PSSEN */
	else if ((offset & 0xc00) == 0x800)
	{
		if (((math.inslatch >> 8) & BB_DSEL) == 3)
		{
			int shift;
			UINT16 val = math.ppshift;

			if (math.cpulatch & 0x3800)
			{
				shift = (math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val = ROR16(val, 1);
					shift >>= 1;
				}
			}
			else
			{
				shift = (math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val = ROL16(val, 1);
					shift >>= 1;
				}
			}
			math.ppshift = val;
		}
		else
		{
			osd_printf_debug("BB_DSEL was not 3 for P->S load!\n");
			debugger_break(machine());
		}
	}
	else
	{
		osd_printf_debug("Buggy Boy unknown math state!\n");
		debugger_break(machine());
	}

	if (offset & BB_INSLD)
	{
		math.promaddr = (offset << 2) & 0x1ff;
		buggyboy_update_state(machine());
	}
	else if (offset & BB_CNTST)
	{
		INC_PROM_ADDR;
		buggyboy_update_state(machine());
	}
}

/*
    This is for ROM range 0x5000-0x7fff
*/
READ16_MEMBER(tx1_state::buggyboy_spcs_rom_r)
{
	math_t &math = m_math;
	math.cpulatch = *(UINT16*)((UINT8*)memregion("math_cpu")->base() + 0x04000 + 0x1000 + offset*2);

	if (math.mux == BB_MUX_ILDEN)
	{
		math.i0ff = math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (math.mux == BB_MUX_MULEN)
	{
		int ins = math.inslatch & 7;

		BB_SET_INS0_BIT;
		kick_sn74s516(machine(), &math.cpulatch, ins);
	}
	else if (math.mux == BB_MUX_PPSEN)
	{
		math.ppshift = math.cpulatch;
	}
	else if (math.mux == BB_MUX_PSSEN)
	{
		if (((math.inslatch >> 8) & BB_DSEL) == 3)
		{
			int shift;
			UINT16 val = math.ppshift;

			if (math.cpulatch & 0x3800)
			{
				shift = (math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val = ROR16(val, 1);
					shift >>= 1;
				}
			}
			else
			{
				shift = (math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val = ROL16(val, 1);
					shift >>= 1;
				}
			}
			math.ppshift = val;
		}
	}

	if (math.mux != BB_MUX_ILDEN)
	{
		INC_PROM_ADDR;
		buggyboy_update_state(machine());
	}

	return math.cpulatch;
}

WRITE16_MEMBER(tx1_state::buggyboy_spcs_ram_w)
{
	COMBINE_DATA(&m_math_ram[offset]);
}

READ16_MEMBER(tx1_state::buggyboy_spcs_ram_r)
{
	math_t &math = m_math;
	math.cpulatch = m_math_ram[offset];

	offset <<= 1;

	if (math.mux == BB_MUX_ILDEN)
	{
		math.i0ff = math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (math.mux == BB_MUX_MULEN)
	{
		int ins = math.inslatch & 7;

		BB_SET_INS0_BIT;
		kick_sn74s516(machine(), &math.cpulatch, ins);
	}
	else if (math.mux == BB_MUX_PPSEN)
	{
		math.ppshift = math.cpulatch;
	}
	else if (math.mux == BB_MUX_PSSEN)
	{
		if (((math.inslatch >> 8) & BB_DSEL) == 3)
		{
			int shift;
			UINT16 val = math.ppshift;

			if (math.cpulatch & 0x3800)
			{
				shift = (math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val = ROR16(val, 1);
					shift >>= 1;
				}
			}
			else
			{
				shift = (math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val = ROL16(val, 1);
					shift >>= 1;
				}
			}
			math.ppshift = val;
		}
	}

	if (math.mux != BB_MUX_ILDEN)
	{
		INC_PROM_ADDR;
		buggyboy_update_state(machine());
	}

	return math.cpulatch;
}



/*************************************
 *
 *  Machine Reset
 *
 *************************************/

MACHINE_RESET_MEMBER(tx1_state,buggyboy)
{
	// TODO: This is connected to the /BUSACK line of the Z80
	m_maincpu->set_input_line(INPUT_LINE_TEST, ASSERT_LINE);

	memset(&m_math, 0, sizeof(m_math));
}

MACHINE_RESET_MEMBER(tx1_state,tx1)
{
	// TODO: This is connected to the /BUSACK line of the Z80
	m_maincpu->set_input_line(INPUT_LINE_TEST, ASSERT_LINE);

	memset(&m_math, 0, sizeof(m_math));
}
