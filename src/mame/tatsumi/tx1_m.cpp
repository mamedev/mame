// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi TX-1/Buggy Boy machine hardware

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cpu/i86/i86.h"
#include "tx1.h"


/*
    Helper functions
*/
#define INC_PROM_ADDR       ( m_math.promaddr = (m_math.promaddr + 1) & 0x1ff )
#define ROR16(val, shift)   ( ((uint16_t)val >> shift) | ((uint16_t)val << (16 - shift)) )
#define ROL16(val, shift)   ( ((uint16_t)val << shift) | ((uint16_t)val >> (16 - shift)) )
#define SWAP16(val)         ( (((uint16_t)val << 8) & 0xff00) | ((uint16_t)val >> 8) )

static inline uint8_t reverse_nibble(uint8_t nibble)
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
static constexpr uint8_t state_table[16][8] =
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

void tx1_state::sn74s516_t::multiply(running_machine &machine)
{
	switch (code)
	{
	case 0:
		ZW.ZW32 = X * Y;
		break;
	case 2:
		ZW.ZW32 += X * Y;
		break;
	case 3:
		ZW.ZW32 += -X * Y;
		break;
	case 0x60:
		ZW.ZW32 = X * Y;
		break;
	case 0x61:
		ZW.ZW32 = -X * Y;
		break;
	case 0x62:
		ZW.ZW32 += X * Y;
		break;
	case 0x63:
		ZW.ZW32 += -X * Y;
		break;
	case 0x660:
		ZW.ZW32 = (X * Y) + (ZW.ZW32 & 0xffff0000);
		break;
	case 0x661:
		ZW.ZW32 = (-X * Y) + (ZW.ZW32 & 0xffff0000);
		break;
	case 0x662:
		ZW.ZW32 = (-X * Y) + (ZW.ZW32 & 0xffff0000);
		break;
	case 0x6660:
		ZW.ZW32 += (X * Y);
		break;
	default:
		osd_printf_debug("sn74s516 ??? multiply: %x\n", code);
	}

	// Seems a good enough place to clear it.
	ZWfl = 0;
}

void tx1_state::sn74s516_t::divide(running_machine &machine)
{
	if (X == 0)
	{
		machine.logerror("%s: SN74S516 tried to divide by zero\n", machine.describe_context());
		ZW.as16bit.Z = int16_t(0xffff);
		ZW.as16bit.W = 0xffff;
		ZWfl = 0;
		return;
	}

	int32_t Z = 0;
	int32_t W = 0;

	switch (code)
	{
	case 4:
		Z = ZW.ZW32 / X;
		W = ZW.ZW32 % X;
		break;
	case 0x664:
		Z = ZW.ZW32 / X;
		W = ZW.ZW32 % X;
		break;
	case 0x6664:
		Z = ZW.as16bit.W / X;
		W = ZW.as16bit.W % X;
		break;
	default:
		osd_printf_debug("SN74S516 unhandled divide type: %x\n", code);
	}

	// Divide overflow Only happens during chip test anyway
	if (Z > 0xffff)
		Z |= 0xff00;

	ZW.as16bit.Z = Z;
	ZW.as16bit.W = W;
	ZWfl = 0;
}

void tx1_state::sn74s516_t::update(running_machine &machine, int ins)
{
	state = state_table[state][ins];

	if (state == 4)
	{
		multiply(machine);
		state = 8;
	}
	else if (state == 5)
	{
		divide(machine);
		state = 10;
	}
}

void tx1_state::sn74s516_t::kick(running_machine &machine, math_t &math, uint16_t *data, int ins)
{
#define LOAD_X      (X = *data)
#define LOAD_Y      (Y = *data)
#define LOAD_Z      (ZW.as16bit.Z = *data)
#define LOAD_W      (ZW.as16bit.W = *data)
#define READ_ZW     *data = ZWfl ? ZW.as16bit.W : ZW.as16bit.Z; \
					ZWfl ^= 1;

#define UPDATE_SEQUENCE (code = (code << 4) | ins)
#define CLEAR_SEQUENCE  (code = 0)

	/*
	    Remember to change the Z/W flag.
	*/
	switch (state)
	{
	case 0:
		CLEAR_SEQUENCE;
		UPDATE_SEQUENCE;

		if (ins < 4)
		{
			LOAD_Y;
			update(machine, ins);
		}
		else if (ins == 4)
		{
			update(machine, ins);
		}
		else if (ins < 7)
		{
			LOAD_X;
			update(machine, ins);
		}
		else if (ins == 7)
		{
			READ_ZW;
			break;
		}

		break;
	case 8:
	case 10:
		CLEAR_SEQUENCE;
		UPDATE_SEQUENCE;

		if (ins < 4)
		{
			LOAD_Y;
			update(machine, ins);
		}
		else if (ins == 4)
		{
			update(machine, ins);
		}
		else if (ins == 5)
		{
			// Rounding
			// Operation
			update(machine, ins);
		}
		else if (ins == 6)
		{
			LOAD_X;
			update(machine, ins);
		}
		else if (ins == 7)
		{
			READ_ZW;
			update(machine, ins);
		}
		break;
	case 1:
		// TODO: 6666 represents an incomplete state - clear it.
		if (code == 0x6666)
		{
			CLEAR_SEQUENCE;
			machine.logerror("%s: Code 6666: PROMADDR:%x\n", machine.describe_context(), math.promaddr);
		}

		UPDATE_SEQUENCE;
		if (ins < 4)
		{
			LOAD_Y;
			update(machine, ins);
		}
		else if (ins < 6)
		{
			update(machine, ins);
		}
		else if (ins == 6)
		{
			LOAD_Z;
			update(machine, ins);
		}
		else if (ins == 7)
		{
			// Pointless operation.
			update(machine, ins);
		}

		break;
	case 3:
		UPDATE_SEQUENCE;
		if (ins < 4)
		{
			LOAD_Y;
			update(machine, ins);
		}
		else if (ins == 4)
		{
			LOAD_W;
			update(machine, ins);
		}
		else if (ins == 5)
		{
			update(machine, ins);
		}
		else if (ins == 6)
		{
			LOAD_W;
			update(machine, ins);
		}
		else if (ins == 7)
		{
			READ_ZW;
			update(machine, ins);
		}
		break;
	case 11:
		UPDATE_SEQUENCE;
		if (ins < 4)
		{
			LOAD_Y;
			update(machine, ins);
		}
		else if (ins < 6)
		{
			update(machine, ins);
		}
		else if (ins == 6)
		{
			// CHECK: Incomplete state
			update(machine, ins);
		}
		else if (ins == 7)
		{
			// 6667 = Load X, Load Z, Load W, Clear Z
			ZW.as16bit.Z = 0;
			update(machine, ins);
		}
		break;
	default:
		osd_printf_debug("Unknown SN74S516 state. %x\n", code);
	}
}

inline void tx1_state::kick_sn74s516(uint16_t *data, int ins)
{
	m_sn74s516.kick(machine(), m_math, data, ins);

	m_math.dbgaddr = m_math.promaddr;
	m_math.dbgpc = m_mathcpu->pcbase();
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

#define TX1_SET_INS0_BIT    do { if (!(ins & 0x4) && m_math.i0ff) ins |= m_math.i0ff; } while(0)

inline uint16_t tx1_state::math_t::get_datarom_addr() const
{
	uint16_t addr = ((inslatch & 0x1c00) << 1) | (ppshift & 0xff);

	if ((inslatch >> 8) & TX1_RADCHG)
		addr |= (ppshift & 0x0700);
	else
		addr |= (promaddr << 3) & 0x0700;

	return addr & 0x3fff;
}

void tx1_state::tx1_update_state()
{
#define LHIEN(a)    !(a & 0x80)
#define LLOEN(a)    !(a & 0x40)
#define GO_EN(a)    !(a & 0x4000)

	const uint16_t *prom = (uint16_t*)memregion("au_data")->base() + (0x8000 >> 1);

	for (;;)
	{
		int go = 0;

		if (!GO_EN(m_math.inslatch) && GO_EN(prom[m_math.promaddr]))
			go = 1;
		/*
		    Example:
		    120 /GO /LHIEN
		    121 /GO        /LLOEN
		    Both 120 and 121 are used.
		*/
		else if ((GO_EN(m_math.inslatch) && GO_EN(prom[m_math.promaddr])) && (LHIEN(m_math.inslatch) && LLOEN(prom[m_math.promaddr])))
			go = 1;

		/* Now update the latch */
		m_math.inslatch = prom[m_math.promaddr] & 0x7fff;
		m_math.mux = (m_math.inslatch >> 3) & 7;

		if (m_math.mux == TX1_SEL_INSCL)
		{
			m_math.i0ff = 0;
		}
		else if (m_math.mux == TX1_SEL_PPSEN)
		{
			// NOTE: Doesn't do anything without SPCS.
		}

		/* TODO */
		if (go)
		{
			int ins = m_math.inslatch & 7;

			TX1_SET_INS0_BIT;

			if (m_math.mux == TX1_SEL_DSELOE)
			{
				int     dsel = (m_math.inslatch >> 8) & TX1_DSEL;
				int     tfad = (m_math.inslatch & 0x1c00) << 1;
				int     sd   = m_math.ppshift;
				int     o4;
				uint16_t  data;

				o4 =
					(!BIT(sd, 9) && !BIT(sd,10)) ||
					( BIT(sd, 7) &&  BIT(sd,10)) ||
					(!BIT(sd, 8) &&  BIT(sd, 9)) ||
					(!BIT(sd, 7) &&  BIT(sd, 8)) ||
					!BIT(dsel, 1) || BIT(tfad, 13) || BIT(tfad, 12) || BIT(tfad, 11);

				dsel = (dsel & 2) | ((dsel & o4) ^ 1);

				if (dsel == 0)
					data = m_math.muxlatch;
				else if (dsel == 1)
				{
					uint16_t *romdata = (uint16_t*)memregion("au_data")->base();
					uint16_t addr = m_math.get_datarom_addr();
					data = romdata[addr];
				}
				else if (dsel == 2)
					data = ROL16(m_math.muxlatch, 4);
				else if (dsel == 3)
					data = ROL16(SWAP16(m_math.muxlatch), 3);

				kick_sn74s516(&data, ins);
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
			else if (LHIEN(m_math.inslatch) || LLOEN(m_math.inslatch))
			{
				uint16_t data;

				kick_sn74s516(&data, ins);

				/* All latches enabled */
				if (LHIEN(m_math.inslatch) && LLOEN(m_math.inslatch))
				{
					m_math.muxlatch = data;
				}
				else if (m_math.mux == TX1_SEL_LMSEL) // O4 = 0
				{
					// TMPLD2/TMPLD3 15-5
					if (LLOEN(m_math.inslatch))
					{
						m_math.muxlatch &= 0x001f;
						m_math.muxlatch |= data & 0xffe0;
					}
					// TMLPD1 4-0???????
					else if (LHIEN(m_math.inslatch))
					{
						m_math.muxlatch &= 0xffe0;
						m_math.muxlatch |= data & 0x001f;
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
					int     dsel = (m_math.inslatch >> 8) & TX1_DSEL;
					int     tfad = (m_math.inslatch & 0x1c00) << 1;
					int     sd   = m_math.ppshift;
					int     o4;

					o4 =
						(!BIT(sd, 9) && !BIT(sd,10)) ||
						( BIT(sd, 7) &&  BIT(sd,10)) ||
						(!BIT(sd, 8) &&  BIT(sd, 9)) ||
						(!BIT(sd, 7) &&  BIT(sd, 8)) ||
						!BIT(dsel, 1) || BIT(tfad, 13) || BIT(tfad, 12) || BIT(tfad, 11);

					if (LLOEN(m_math.inslatch))
					{
						m_math.muxlatch &= 0x0fff;
						m_math.muxlatch |= data & 0xf000;

						if (!o4)
						{
							// TMPLD11-5
							m_math.muxlatch &= 0xf01f;
							m_math.muxlatch |= data & 0x0fe0;
						}
					}
					else if (LHIEN(m_math.inslatch))
					{
						m_math.muxlatch &= 0xffe0;
						m_math.muxlatch |= data & 0x001f;

						if (o4)
						{
							// TMPLD11-5
							m_math.muxlatch &= 0xf01f;
							m_math.muxlatch |= data & 0x0fe0;
						}
					}
				}
			}
			else
			{
				if (m_math.mux == TX1_SEL_PPSEN)
				{
					kick_sn74s516(&m_math.ppshift, ins);
				}
				else
				{
					/* Bus pullups give 0xffff */
					uint16_t data = 0xffff;
					kick_sn74s516(&data, ins);
				}
			}
		}

		/* Is there another instruction in the sequence? */
		if (prom[m_math.promaddr] & 0x8000)
			break;
		else
			INC_PROM_ADDR;
	}
}

uint16_t tx1_state::tx1_math_r(offs_t offset)
{
	offset = offset << 1;

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = m_math.inslatch & 7;
			TX1_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		/* TODO What do we return? */
		kick_sn74s516(&m_math.retval, ins);
	}
	/* /PPSEN */
	else if (offset < 0x800)
	{
		// Unused - just pullups?
		m_math.retval = 0xffff;
	}
	/* /MUXCS */
	else if ((offset & 0xc00) == 0xc00)
	{
		int     dsel = (m_math.inslatch >> 8) & TX1_DSEL;
		int     tfad = (m_math.inslatch & 0x1c00) << 1;
		int     sd   = m_math.ppshift;
		int     o4;

		if (m_math.mux == TX1_SEL_LMSEL)
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
			m_math.retval = m_math.muxlatch;
		else if (dsel == 1 )
		{
			/*
			    TODO make this constant somewhere
			    e.g. m_math.retval =  m_math.romptr[ m_math.get_datarom_addr() ];
			*/
			uint16_t *romdata = (uint16_t*)memregion("au_data")->base();
			uint16_t addr = m_math.get_datarom_addr();
			m_math.retval = romdata[addr];
		}
		else if (dsel == 2)
			m_math.retval = ROL16(m_math.muxlatch, 4);
		else if (dsel == 3)
			m_math.retval = ROL16(SWAP16(m_math.muxlatch), 3);

		/* TODO for TX-1: This is /SPCS region? */
		if (offset < 0xe00)
		{
			// Load the PP with retval??????
			if (m_math.mux == TX1_SEL_PPSEN)
			{
				m_math.ppshift = m_math.retval & 0x3fff;
			}
			else if (m_math.mux == TX1_SEL_PSSEN)
			{
				// WRONG!!!!
				osd_printf_debug("Math Read with PSSEN!\n");
				m_math.ppshift = m_math.retval;
			}

			if (m_math.mux != TX1_SEL_ILDEN)
			{
				INC_PROM_ADDR;
				tx1_update_state();

				// MUST RETURN HERE?
				return m_math.retval;
			}
		}
	}
	else
	{
		if (m_math.mux == TX1_SEL_PPSEN)
			m_math.retval = m_math.ppshift & 0x3fff;
		else
			/* Nothing is mapped - read from pull up resistors! */
			m_math.retval = 0xffff;
	}

	if (offset & TX1_INSLD)
	{
		m_math.promaddr = (offset << 2) & 0x1ff;
		tx1_update_state();
	}
	else if (offset & TX1_CNTST)
	{
		INC_PROM_ADDR;
		tx1_update_state();
	}

	return m_math.retval;
}

void tx1_state::tx1_math_w(offs_t offset, uint16_t data)
{
	m_math.cpulatch = data;
	offset <<= 1;

//  printf("W %x: %x\n", 0x3000 + offset, data);

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = m_math.inslatch & 7;
			TX1_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		kick_sn74s516(&m_math.cpulatch, ins);
	}
	/* /PPSEN */
	else if ((offset & 0xc00) == 0x400)
	{
		/* Input is 14 bits */
		m_math.ppshift = m_math.cpulatch & 0x3fff;
	}
	/* /PSSEN */
	else if ((offset & 0xc00) == 0x800)
	{
		//if (((m_math.inslatch >> 8) & TX1_DSEL) == 3 )
		{
			int shift;
			uint16_t val = m_math.ppshift;

			if (m_math.cpulatch & 0x3800)
			{
				shift = (m_math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val >>= 1;
					shift >>= 1;
				}
			}
			else
			{
				shift = (m_math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val <<= 1;
					shift >>= 1;
				}
			}
			m_math.ppshift = val;
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
		m_math.muxlatch = m_math.cpulatch;
	}

	if (offset & TX1_INSLD)
	{
		m_math.promaddr = (offset << 2) & 0x1ff;
		tx1_update_state();
	}
	else if (offset & TX1_CNTST)
	{
		INC_PROM_ADDR;
		tx1_update_state();
	}
}

uint16_t tx1_state::tx1_spcs_rom_r(offs_t offset)
{
	m_math.cpulatch = *(uint16_t*)((uint8_t*)memregion("math_cpu")->base() + 0x04000 + 0x1000 + offset*2);

	if (m_math.mux == TX1_SEL_ILDEN)
	{
		m_math.i0ff = m_math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (m_math.mux == TX1_SEL_MULEN)
	{
		int ins = m_math.inslatch & 7;

		TX1_SET_INS0_BIT;
		kick_sn74s516(&m_math.cpulatch, ins);
	}
	else if (m_math.mux == TX1_SEL_PPSEN)
	{
		m_math.ppshift = m_math.cpulatch;
	}
	else if (m_math.mux == TX1_SEL_PSSEN)
	{
			//if ( ((m_math.inslatch >> 8) & TX1_DSEL) == 3 )
		{
			int shift;
			uint16_t val = m_math.ppshift;

			if (m_math.cpulatch & 0x3800)
			{
				shift = (m_math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val >>= 1;
					shift >>= 1;
				}
			}
			else
			{
				shift = (m_math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val <<= 1;
					shift >>= 1;
				}
			}
			m_math.ppshift = val & 0x7ff;
		}
	}

	if (m_math.mux != TX1_SEL_ILDEN)
	{
		INC_PROM_ADDR;
		tx1_update_state();
	}

	return m_math.cpulatch;

}

uint16_t tx1_state::tx1_spcs_ram_r(offs_t offset)
{
	m_math.cpulatch = m_math_ram[offset];

	offset <<= 1;

	if (m_math.mux == TX1_SEL_ILDEN)
	{
		m_math.i0ff = m_math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (m_math.mux == TX1_SEL_MULEN)
	{
		int ins = m_math.inslatch & 7;

		TX1_SET_INS0_BIT;
		kick_sn74s516(&m_math.cpulatch, ins);
	}
	else if (m_math.mux == TX1_SEL_PPSEN)
	{
//      m_math.ppshift = m_math.retval & 0x3fff;
		m_math.ppshift = m_math.cpulatch;
	}
	else if (m_math.mux == TX1_SEL_PSSEN)
	{
		int shift;
		uint16_t val = m_math.ppshift;

		if (m_math.cpulatch & 0x3800)
		{
			shift = (m_math.cpulatch >> 11) & 0x7;

			while (shift)
			{
				val >>= 1;
				shift >>= 1;
			}
		}
		else
		{
			shift = (m_math.cpulatch >> 7) & 0xf;
			shift = reverse_nibble(shift);
			shift >>= 1;

			while (shift)
			{
				val <<= 1;
				shift >>= 1;
			}
		}
		m_math.ppshift = val & 0x7ff;
	}

	if (m_math.mux != TX1_SEL_ILDEN)
	{
		INC_PROM_ADDR;
		tx1_update_state();
	}

	return m_math.cpulatch;
}

/* Should never occur */
void tx1_state::tx1_spcs_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

#define BB_SET_INS0_BIT do { if (!(ins & 0x4) && m_math.i0ff) ins |= m_math.i0ff;} while(0)

inline uint16_t tx1_state::math_t::get_bb_datarom_addr() const
{
	uint16_t addr = ((inslatch & 0x1c00) << 1) | (ppshift & 0xff);

	if ((inslatch >> 8) & BB_RADCHG)
		addr |= (ppshift & 0x0700);
	else
		addr |= (promaddr << 3) & 0x0700;

	return addr & 0x3fff;
}

void tx1_state::buggyboy_update_state()
{
#define LHIEN(a)    !(a & 0x80)
#define LLOEN(a)    !(a & 0x40)
#define GO_EN(a)    !(a & 0x4000)

	const uint16_t *prom = (uint16_t*)memregion("au_data")->base() + (0x8000 >> 1);

	for (;;)
	{
		int go = 0;

		if (!GO_EN(m_math.inslatch) && GO_EN(prom[m_math.promaddr]))
			go = 1;
		else if ((GO_EN(m_math.inslatch) && GO_EN(prom[m_math.promaddr])) && (LHIEN(m_math.inslatch) && LLOEN(prom[m_math.promaddr])))
			go = 1;

		/* Now update the latch */
		m_math.inslatch = prom[m_math.promaddr] & 0x7fff;
		m_math.mux = (m_math.inslatch >> 3) & 7;

		if (m_math.mux == BB_MUX_INSCL)
			m_math.i0ff = 0;
		else if (m_math.mux == BB_MUX_PPSEN)
		{
			// TODO: Needed?
			//osd_printf_debug("/PPSEN with INS: %x\n", m_math.promaddr);
			//m_math.ppshift = lastval;//m_math.cpulatch;
		}

		/* TODO */
		if (go)
		{
			int ins = m_math.inslatch & 7;

			BB_SET_INS0_BIT;

			if (m_math.mux == BB_MUX_DPROE)
			{
				uint16_t *romdata = (uint16_t*)memregion("au_data")->base();
				uint16_t addr = m_math.get_bb_datarom_addr();
				kick_sn74s516(&romdata[addr], ins);
			}
			else if (m_math.mux == BB_MUX_PPOE)
			{
				kick_sn74s516(&m_math.ppshift, ins);
			}
			/* This is quite tricky. */
			/* It can either be a read operation or */
			/* What if /LHIEN and /LLOEN? */
			else if (LHIEN(m_math.inslatch) || LLOEN(m_math.inslatch))
			{
				uint16_t data;

				kick_sn74s516(&data, ins);

				if (LHIEN(m_math.inslatch) && LLOEN(m_math.inslatch))
				{
					m_math.ppshift = data;
				}
				else if (m_math.mux == BB_MUX_LMSEL)
				{
					if (LLOEN(m_math.inslatch))
					{
						m_math.ppshift &= 0x000f;
						m_math.ppshift |= data & 0xfff0;
					}
					else if (LHIEN(m_math.inslatch))
					{
						m_math.ppshift &= 0xfff0;
						m_math.ppshift |= data & 0x000f;
					}
				}
				else
				{
					if (LLOEN(m_math.inslatch))
					{
						m_math.ppshift &= 0x0fff;
						m_math.ppshift |= data & 0xf000;
					}
					else if (LHIEN(m_math.inslatch))
					{
						m_math.ppshift &= 0xf000;
						m_math.ppshift |= data & 0x0fff;
					}
				}
			}
			else
			{
				if (m_math.mux == BB_MUX_PPSEN)
				{
					kick_sn74s516(&m_math.ppshift, ins);
				}
				else
				{
					/* Bus pullups give 0xffff */
					uint16_t data = 0xffff;
					kick_sn74s516(&data, ins);
				}
			}
		}

		/* Handle rotation */
		if (((m_math.inslatch >> 8) & BB_DSEL) == 1)
		{
			m_math.ppshift = ROR16(m_math.ppshift, 4);
		}
		else if (((m_math.inslatch >> 8) & BB_DSEL) == 2)
		{
			m_math.ppshift = ROL16(m_math.ppshift, 4);
		}

		/* Is there another instruction in the sequence? */
		if (prom[m_math.promaddr] & 0x8000)
			break;
		else
			INC_PROM_ADDR;
	}
}

uint16_t tx1_state::buggyboy_math_r(offs_t offset)
{
	offset = offset << 1;

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = m_math.inslatch & 7;
			BB_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		/* TODO What do we return? */
		kick_sn74s516(&m_math.retval, ins);

		/* TODO */
		//if (m_math.mux == BB_MUX_PPSEN)
		//  m_math.ppshift = m_math.retval;
	}
	/* /PPSEN */
	else if (offset < 0x800)
	{
		m_math.retval = m_math.ppshift;
	}
	/* /DPROE */
	else if ((offset & 0xc00) == 0xc00)
	{
		uint16_t *romdata = (uint16_t*)memregion("au_data")->base();
		uint16_t addr = m_math.get_bb_datarom_addr();

		m_math.retval = romdata[addr];

		/* This is necessary */
		if (m_math.mux == BB_MUX_PPSEN)
			m_math.ppshift = romdata[addr];

		/* This is /SPCS region? Necessary anyway */
		if (offset < 0xe00)
		{
			if (m_math.mux != BB_MUX_ILDEN)
			{
				INC_PROM_ADDR;
				buggyboy_update_state();
			}
		}
	}
	else
	{
		if (m_math.mux == BB_MUX_PPSEN)
			m_math.retval = m_math.ppshift;
		else
			/* Nothing is mapped - read from pull up resistors! */
			m_math.retval = 0xffff;
	}

	if (offset & BB_INSLD)
	{
		m_math.promaddr = (offset << 2) & 0x1ff;
		buggyboy_update_state();
	}
	else if (offset & BB_CNTST)
	{
		INC_PROM_ADDR;
		buggyboy_update_state();
	}

	return m_math.retval;
}

void tx1_state::buggyboy_math_w(offs_t offset, uint16_t data)
{
	m_math.cpulatch = data;

	offset <<= 1;

	/* /MLPCS */
	if (offset < 0x400)
	{
		int ins;

		if (offset & 0x200)
		{
			ins = m_math.inslatch & 7;
			BB_SET_INS0_BIT;
		}
		else
		{
			ins = (offset >> 1) & 7;
		}

		kick_sn74s516(&m_math.cpulatch, ins);
	}
	/* /PPSEN */
	else if ((offset & 0xc00) == 0x400)
	{
		m_math.ppshift = m_math.cpulatch;
	}
	/* /PSSEN */
	else if ((offset & 0xc00) == 0x800)
	{
		if (((m_math.inslatch >> 8) & BB_DSEL) == 3)
		{
			int shift;
			uint16_t val = m_math.ppshift;

			if (m_math.cpulatch & 0x3800)
			{
				shift = (m_math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val = ROR16(val, 1);
					shift >>= 1;
				}
			}
			else
			{
				shift = (m_math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val = ROL16(val, 1);
					shift >>= 1;
				}
			}
			m_math.ppshift = val;
		}
		else
		{
			osd_printf_debug("BB_DSEL was not 3 for P->S load!\n");
			machine().debug_break();
		}
	}
	else
	{
		osd_printf_debug("Buggy Boy unknown math state!\n");
		machine().debug_break();
	}

	if (offset & BB_INSLD)
	{
		m_math.promaddr = (offset << 2) & 0x1ff;
		buggyboy_update_state();
	}
	else if (offset & BB_CNTST)
	{
		INC_PROM_ADDR;
		buggyboy_update_state();
	}
}

/*
    This is for ROM range 0x5000-0x7fff
*/
uint16_t tx1_state::buggyboy_spcs_rom_r(offs_t offset)
{
	m_math.cpulatch = *(uint16_t*)((uint8_t*)memregion("math_cpu")->base() + 0x04000 + 0x1000 + offset*2);

	if (m_math.mux == BB_MUX_ILDEN)
	{
		m_math.i0ff = m_math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (m_math.mux == BB_MUX_MULEN)
	{
		int ins = m_math.inslatch & 7;

		BB_SET_INS0_BIT;
		kick_sn74s516(&m_math.cpulatch, ins);
	}
	else if (m_math.mux == BB_MUX_PPSEN)
	{
		m_math.ppshift = m_math.cpulatch;
	}
	else if (m_math.mux == BB_MUX_PSSEN)
	{
		if (((m_math.inslatch >> 8) & BB_DSEL) == 3)
		{
			int shift;
			uint16_t val = m_math.ppshift;

			if (m_math.cpulatch & 0x3800)
			{
				shift = (m_math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val = ROR16(val, 1);
					shift >>= 1;
				}
			}
			else
			{
				shift = (m_math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val = ROL16(val, 1);
					shift >>= 1;
				}
			}
			m_math.ppshift = val;
		}
	}

	if (m_math.mux != BB_MUX_ILDEN)
	{
		INC_PROM_ADDR;
		buggyboy_update_state();
	}

	return m_math.cpulatch;
}

void tx1_state::buggyboy_spcs_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_math_ram[offset]);
}

uint16_t tx1_state::buggyboy_spcs_ram_r(offs_t offset)
{
	m_math.cpulatch = m_math_ram[offset];

	offset <<= 1;

	if (m_math.mux == BB_MUX_ILDEN)
	{
		m_math.i0ff = m_math.cpulatch & (1 << 14) ? 1 : 0;
	}
	else if (m_math.mux == BB_MUX_MULEN)
	{
		int ins = m_math.inslatch & 7;

		BB_SET_INS0_BIT;
		kick_sn74s516(&m_math.cpulatch, ins);
	}
	else if (m_math.mux == BB_MUX_PPSEN)
	{
		m_math.ppshift = m_math.cpulatch;
	}
	else if (m_math.mux == BB_MUX_PSSEN)
	{
		if (((m_math.inslatch >> 8) & BB_DSEL) == 3)
		{
			int shift;
			uint16_t val = m_math.ppshift;

			if (m_math.cpulatch & 0x3800)
			{
				shift = (m_math.cpulatch >> 11) & 0x7;

				while (shift)
				{
					val = ROR16(val, 1);
					shift >>= 1;
				}
			}
			else
			{
				shift = (m_math.cpulatch >> 7) & 0xf;
				shift = reverse_nibble(shift);
				shift >>= 1;

				while (shift)
				{
					val = ROL16(val, 1);
					shift >>= 1;
				}
			}
			m_math.ppshift = val;
		}
	}

	if (m_math.mux != BB_MUX_ILDEN)
	{
		INC_PROM_ADDR;
		buggyboy_update_state();
	}

	return m_math.cpulatch;
}



/*************************************
 *
 *  Machine Reset
 *
 *************************************/

void tx1_state::machine_reset()
{
	// TODO: This is connected to the /BUSACK line of the Z80
	m_maincpu->set_input_line(INPUT_LINE_TEST, ASSERT_LINE);

	m_math = math_t();

	m_sn74s516.state = 0;
}
