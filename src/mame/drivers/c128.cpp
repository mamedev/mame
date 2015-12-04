// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - connect CAPS LOCK to charom A12 on international variants

*/

#include "includes/c128.h"
#include "bus/cbmiec/c1571.h"
#include "bus/cbmiec/c1581.h"
#include "cpu/z80/z80.h"
#include "machine/cbm_snqk.h"
#include "sound/dac.h"
#include "softlist.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define A11 BIT(offset, 11)
#define A10 BIT(offset, 10)
#define VMA5 BIT(vma, 13)
#define VMA4 BIT(vma, 12)

enum
{
	PLA_OUT_SDEN = 0,
	PLA_OUT_ROM4 = 1,
	PLA_OUT_ROM2 = 2,
	PLA_OUT_DIR = 3,
	PLA_OUT_ROML = 4,
	PLA_OUT_ROMH = 5,
	PLA_OUT_CLRBANK = 6,
	PLA_OUT_FROM1 = 7,
	PLA_OUT_ROM3 = 8,
	PLA_OUT_ROM1 = 9,
	PLA_OUT_IOCS = 10,
	PLA_OUT_DWE = 11,
	PLA_OUT_CASENB = 12,
	PLA_OUT_VIC = 13,
	PLA_OUT_IOACC = 14,
	PLA_OUT_GWE = 15,
	PLA_OUT_COLORRAM = 16,
	PLA_OUT_CHAROM = 17
};


QUICKLOAD_LOAD_MEMBER( c128_state, cbm_c64 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

inline void c128_state::check_interrupts()
{
	int irq = m_cia1_irq || m_vic_irq || m_exp_irq;
	int nmi = m_cia2_irq || !m_restore || m_exp_nmi;
	//int aec = m_exp_dma && m_z80_busack;
	//int rdy = m_vic_aec && m_z80en && m_vic_ba;
	//int busreq = !m_z80en || !(m_z80_busack && !aec)

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, irq);

	m_subcpu->set_input_line(M8502_IRQ_LINE, irq);
	m_subcpu->set_input_line(M8502_NMI_LINE, nmi);
}



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla -
//-------------------------------------------------

int c128_state::read_pla(offs_t offset, offs_t ca, offs_t vma, int ba, int rw, int aec, int z80io, int ms3, int ms2, int ms1, int ms0)
{
	int _128_256 = 1;
	int dmaack = 1;
	int vicfix = 1;
	int sphi2 = m_vic->phi0_r();

	m_game = m_exp->game_r(ca, sphi2, ba, rw, m_hiram);
	m_exrom = m_exp->exrom_r(ca, sphi2, ba, rw, m_hiram);

	UINT32 input = sphi2 << 26 | m_va14 << 25 | m_charen << 24 |
		m_hiram << 23 | m_loram << 22 | ba << 21 | VMA5 << 20 | VMA4 << 19 | ms0 << 18 | ms1 << 17 | ms2 << 16 |
		m_exrom << 15 | m_game << 14 | rw << 13 | aec << 12 | A10 << 11 | A11 << 10 | A12 << 9 | A13 << 8 |
		A14 << 7 | A15 << 6 | z80io << 5 | m_z80en << 4 | ms3 << 3 | vicfix << 2 | dmaack << 1 | _128_256;

	return m_pla->read(input);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 c128_state::read_memory(address_space &space, offs_t offset, offs_t vma, int ba, int aec, int z80io)
{
	int rw = 1, ms0 = 1, ms1 = 1, ms2 = 1, ms3 = 1, cas0 = 1, cas1 = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	UINT8 data = 0xff;

	offs_t ta = m_mmu->ta_r(offset, aec, &ms0, &ms1, &ms2, &ms3, &cas0, &cas1);
	offs_t ma = 0;
	offs_t sa = 0;

	if (aec)
	{
		data = m_vic->bus_r();
		ma = ta | (offset & 0xff);
		sa = offset & 0xff;
	}
	else
	{
		ta &= ~0xf00;
		ta |= (vma & 0xf00);
		ma = (!m_va15 << 15) | (!m_va14 << 14) | vma;
		sa = vma & 0xff;
	}

	offs_t ca = ta | sa;

	int plaout = read_pla(offset, ca, vma, ba, rw, aec, z80io, ms3, ms2, ms1, ms0);

	m_clrbank = BIT(plaout, PLA_OUT_CLRBANK);

	if (!BIT(plaout, PLA_OUT_CASENB))
	{
		if (!cas0)
		{
			data = m_ram->pointer()[ma];
		}
		if (!cas1)
		{
			data = m_ram->pointer()[0x10000 | ma];
		}
	}
	if (!BIT(plaout, PLA_OUT_ROM1))
	{
		// CR: data = m_rom1[(ms3 << 14) | ((BIT(ta, 14) && BIT(offset, 13)) << 13) | (ta & 0x1000) | (offset & 0xfff)];
		data = m_rom->base()[((BIT(ta, 14) && BIT(offset, 13)) << 13) | (ta & 0x1000) | (offset & 0xfff)];
	}
	if (!BIT(plaout, PLA_OUT_ROM2))
	{
		data = m_rom->base()[0x4000 | (offset & 0x3fff)];
	}
	if (!BIT(plaout, PLA_OUT_ROM3))
	{
		// CR: data = m_rom3[(BIT(offset, 15) << 14) | (offset & 0x3fff)];
		data = m_rom->base()[0x8000 | (offset & 0x3fff)];
	}
	if (!BIT(plaout, PLA_OUT_ROM4))
	{
		data = m_rom->base()[0xc000 | (ta & 0x1000) | (offset & 0x2fff)];
	}
	if (!BIT(plaout, PLA_OUT_CHAROM))
	{
		data = m_charom->base()[(ms3 << 12) | (ta & 0xf00) | sa];
	}
	if (!BIT(plaout, PLA_OUT_COLORRAM) && aec)
	{
		data = m_color_ram[(m_clrbank << 10) | (ta & 0x300) | sa] & 0x0f;
	}
	if (!BIT(plaout, PLA_OUT_VIC))
	{
		data = m_vic->read(space, offset & 0x3f);
	}
	if (!BIT(plaout, PLA_OUT_FROM1) && m_from->exists())
	{
		data = m_from->read_rom(space, offset & 0x7fff);
	}
	if (!BIT(plaout, PLA_OUT_IOCS) && BIT(offset, 10))
	{
		switch ((BIT(offset, 11) << 2) | ((offset >> 8) & 0x03))
		{
		case 0: // SID
			data = m_sid->read(space, offset & 0x1f);
			break;

		case 2: // CS8563
			if BIT(offset, 0)
			{
				data = m_vdc->register_r(space, 0);
			}
			else
			{
				data = m_vdc->status_r(space, 0);
			}
			break;

		case 4: // CIA1
			data = m_cia1->read(space, offset & 0x0f);
			break;

		case 5: // CIA2
			data = m_cia2->read(space, offset & 0x0f);
			break;

		case 6: // I/O1
			io1 = 0;
			break;

		case 7: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);

	data = m_exp->cd_r(space, ca, data, sphi2, ba, roml, romh, io1, io2);

	return m_mmu->read(offset, data);
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void c128_state::write_memory(address_space &space, offs_t offset, offs_t vma, UINT8 data, int ba, int aec, int z80io)
{
	int rw = 0, ms0 = 1, ms1 = 1, ms2 = 1, ms3 = 1, cas0 = 1, cas1 = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	offs_t ta = m_mmu->ta_r(offset, aec, &ms0, &ms1, &ms2, &ms3, &cas0, &cas1);
	offs_t ca = ta | (offset & 0xff);
	offs_t ma = ta | (offset & 0xff);
	offs_t sa = offset & 0xff;

	int plaout = read_pla(offset, ca, vma, ba, rw, aec, z80io, ms3, ms2, ms1, ms0);

	m_clrbank = BIT(plaout, PLA_OUT_CLRBANK);

	if (!BIT(plaout, PLA_OUT_CASENB) && !BIT(plaout, PLA_OUT_DWE))
	{
		if (!cas0)
		{
			m_ram->pointer()[ma] = data;
		}
		if (!cas1)
		{
			m_ram->pointer()[0x10000 | ma] = data;
		}
	}
	if (!BIT(plaout, PLA_OUT_COLORRAM) && !BIT(plaout, PLA_OUT_GWE))
	{
		m_color_ram[(m_clrbank << 10) | (ta & 0x300) | sa] = data & 0x0f;
	}
	if (!BIT(plaout, PLA_OUT_VIC))
	{
		m_vic->write(space, offset & 0x3f, data);
	}
	if (!BIT(plaout, PLA_OUT_IOCS) && BIT(offset, 10))
	{
		switch ((BIT(offset, 11) << 2) | ((offset >> 8) & 0x03))
		{
		case 0: // SID
			m_sid->write(space, offset & 0x1f, data);
			break;

		case 2: // CS8563
			if BIT(offset, 0)
			{
				m_vdc->register_w(space, 0, data);
			}
			else
			{
				m_vdc->address_w(space, 0, data);
			}
			break;

		case 4: // CIA1
			m_cia1->write(space, offset & 0x0f, data);
			break;

		case 5: // CIA2
			m_cia2->write(space, offset & 0x0f, data);
			break;

		case 6: // I/O1
			io1 = 0;
			break;

		case 7: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);

	m_exp->cd_w(space, ca, data, sphi2, ba, roml, romh, io1, io2);

	m_mmu->write(space, offset, data);
}


//-------------------------------------------------
//  z80_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::z80_r )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_w -
//-------------------------------------------------

WRITE8_MEMBER( c128_state::z80_w )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	write_memory(space, offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_io_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::z80_io_r )
{
	int ba = 1, aec = 1, z80io = 0;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  z80_io_w -
//-------------------------------------------------

WRITE8_MEMBER( c128_state::z80_io_w )
{
	int ba = 1, aec = 1, z80io = 0;
	offs_t vma = 0;

	write_memory(space, offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( c128_state::read )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( c128_state::write )
{
	int ba = 1, aec = 1, z80io = 1;
	offs_t vma = 0;

	write_memory(space, offset, vma, data, ba, aec, z80io);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::vic_videoram_r )
{
	int ba = 0, aec = 0, z80io = 1;

	return read_memory(space, 0, offset, ba, aec, z80io);
}


//-------------------------------------------------
//  vic_colorram_r -
//-------------------------------------------------

READ8_MEMBER( c128_state::vic_colorram_r )
{
	return m_color_ram[(m_clrbank << 10) | offset];
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_mem, AS_PROGRAM, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(z80_r, z80_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_io, AS_IO, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(z80_io_r, z80_io_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m8502_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m8502_mem, AS_PROGRAM, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, c128_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, c128_state )
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_colorram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vdc_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vdc_videoram_map, AS_0, 8, c128_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c128 )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::write_restore )
{
	m_restore = state;

	check_interrupts();
}

INPUT_CHANGED_MEMBER( c128_state::caps_lock )
{
	m_caps_lock = newval;
}

static INPUT_PORTS_START( c128 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR HOME") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR('\xA3')

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "K0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_F6)               PORT_CHAR('\t')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START( "K1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_F5)               PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START( "K2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NO SCROLL") PORT_CODE(KEYCODE_F12) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)             PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)              PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)              PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c128_state, write_restore)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)

	PORT_START( "40_80" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("40/80 DISPLAY") PORT_CODE(KEYCODE_F11) PORT_TOGGLE
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128_de )
//-------------------------------------------------

static INPUT_PORTS_START( c128_de )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z  { Y }") PORT_CODE(KEYCODE_Z)                   PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y  { Z }") PORT_CODE(KEYCODE_Y)                   PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  { = }") PORT_CODE(KEYCODE_0)                   PORT_CHAR('0')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(",  <  { ; }") PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Paragraph  \xE2\x86\x91  { \xc3\xbc }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00A7) PORT_CHAR(0x2191)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".  >  { : }") PORT_CODE(KEYCODE_STOP)             PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-  { '  ` }") PORT_CODE(KEYCODE_EQUALS)           PORT_CHAR('-')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+  { \xc3\x9f ? }") PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/  ?  { -  _ }") PORT_CODE(KEYCODE_SLASH)                 PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Sum  Pi  { ] \\ }") PORT_CODE(KEYCODE_DEL)                PORT_CHAR(0x03A3) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  { # ' }") PORT_CODE(KEYCODE_BACKSLASH)                 PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  ]  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE)             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("*  `  { +  * }") PORT_CODE(KEYCODE_CLOSEBRACE)            PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\  { [  \xE2\x86\x91 }") PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)                    PORT_CHAR('_')

	PORT_MODIFY( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ASCII/DIN") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c128_fr )
//-------------------------------------------------
#ifdef UNUSED_CODE
static INPUT_PORTS_START( c128_fr )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)               PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A  { Q }") PORT_CODE(KEYCODE_A)               PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)               PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6  &  { Paragraph  6 }") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8  (  { !  8 }") PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K  Large-  { \\ }") PORT_CODE(KEYCODE_K)      PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)   PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(",  <  { ;  . }") PORT_CODE(KEYCODE_COMMA)                     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { ^  \xc2\xa8 }") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)              PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)                      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  _ }") PORT_CODE(KEYCODE_EQUALS)             PORT_CHAR('-') PORT_CHAR('\xB0')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/  ?  { =  + }") PORT_CODE(KEYCODE_SLASH)                     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)           PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  {\xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH)         PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  ]  { M  Large-/ }") PORT_CODE(KEYCODE_QUOTE)               PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)                PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH)                   PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q  { A }") PORT_CODE(KEYCODE_Q)               PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_   { <  > }") PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1  !  { &  1 }") PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')

	PORT_MODIFY( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK ASCII/CC") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END
#endif

//-------------------------------------------------
//  INPUT_PORTS( c128_it )
//-------------------------------------------------
#ifdef UNUSED_CODE
static INPUT_PORTS_START( c128_it )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z  { W }") PORT_CODE(KEYCODE_Z)                       PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4  $  { '  4 }") PORT_CODE(KEYCODE_4)                 PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W  { Z }") PORT_CODE(KEYCODE_W)                       PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { \"  3 }") PORT_CODE(KEYCODE_3)                PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6  &  { _  6 }") PORT_CODE(KEYCODE_6)                 PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5  %  { (  5 }") PORT_CODE(KEYCODE_5)                 PORT_CHAR('5') PORT_CHAR('%')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8  (  { &  8 }") PORT_CODE(KEYCODE_8)                 PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { \xc3\xa8  7 }") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M  Large-/  { ,  ? }") PORT_CODE(KEYCODE_M)           PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  { \xc3\xa0  0 }") PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9  )  { \xc3\xa7  9 }") PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(",  <   { ;  . }") PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@  \xc3\xbb  { \xc3\xac  = }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(0x00FB)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xb9  % }") PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".  >  { :  / }") PORT_CODE(KEYCODE_STOP)              PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-  \xc2\xb0  { -  + }") PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR('\xb0')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+  \xc3\xab  { )  \xc2\xb0 }") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR(0x00EB)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/  ?  { \xc3\xb2  ! }") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi  { *  ] }") PORT_CODE(KEYCODE_DEL)   PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  { \xE2\x86\x91  \\ }") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  ]  { M }") PORT_CODE(KEYCODE_QUOTE)                PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("*  `  { $  [ }") PORT_CODE(KEYCODE_CLOSEBRACE)        PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\  { @  # }") PORT_CODE(KEYCODE_BACKSLASH2)          PORT_CHAR('\\')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2  \"  { \xc3\xa9  2 }") PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)                PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1  !  { \xc2\xa3  1 }") PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
INPUT_PORTS_END
#endif

//-------------------------------------------------
//  INPUT_PORTS( c128_se )
//-------------------------------------------------

static INPUT_PORTS_START( c128_se )
	PORT_INCLUDE( c128 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7  '  { 7  / }") PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("]  { \xc3\xa2 }") PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("[  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)        PORT_CHAR('[')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                    PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                                     PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";  +") PORT_CODE(KEYCODE_BACKSLASH)               PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xc2\xa3  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\xA3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_CLOSEBRACE)                 PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(":  *") PORT_CODE(KEYCODE_BACKSLASH2)              PORT_CHAR(':') PORT_CHAR('*')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)            PORT_CHAR('_')

	PORT_MODIFY( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK ASCII/CC") PORT_CODE(KEYCODE_F8) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, c128_state, caps_lock, 0)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MOS8722_INTERFACE( mmu_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::mmu_z80en_w )
{
	if (state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

		if (m_reset)
		{
			m_subcpu->reset();

			m_reset = 0;
		}
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	m_z80en = state;
}

WRITE_LINE_MEMBER( c128_state::mmu_fsdir_w )
{
	update_iec();
}

READ_LINE_MEMBER( c128_state::mmu_game_r )
{
	return m_game;
}

READ_LINE_MEMBER( c128_state::mmu_exrom_r )
{
	return m_exrom;
}

READ_LINE_MEMBER( c128_state::mmu_sense40_r )
{
	return BIT(m_40_80->read(), 0);
}


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

static GFXDECODE_START( c128 )
	GFXDECODE_ENTRY( "charom", 0x0000, gfx_8x8x1, 0, 1 )
GFXDECODE_END


//-------------------------------------------------
//  MOS8564_INTERFACE( vic_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::vic_irq_w )
{
	m_vic_irq = state;

	check_interrupts();
}

WRITE8_MEMBER( c128_state::vic_k_w )
{
	m_vic_k = data;
}


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::sid_potx_r )
{
	UINT8 data = 0xff;

	switch (m_cia1->pa_r() >> 6)
	{
	case 1: data = m_joy1->pot_x_r(); break;
	case 2: data = m_joy2->pot_x_r(); break;
	case 3:
		if (m_joy1->has_pot_x() && m_joy2->has_pot_x())
		{
			data = 1 / (1 / m_joy1->pot_x_r() + 1 / m_joy2->pot_x_r());
		}
		else if (m_joy1->has_pot_x())
		{
			data = m_joy1->pot_x_r();
		}
		else if (m_joy2->has_pot_x())
		{
			data = m_joy2->pot_x_r();
		}
		break;
	}

	return data;
}

READ8_MEMBER( c128_state::sid_poty_r )
{
	UINT8 data = 0xff;

	switch (m_cia1->pa_r() >> 6)
	{
	case 1: data = m_joy1->pot_y_r(); break;
	case 2: data = m_joy2->pot_y_r(); break;
	case 3:
		if (m_joy1->has_pot_y() && m_joy2->has_pot_y())
		{
			data = 1 / (1 / m_joy1->pot_y_r() + 1 / m_joy2->pot_y_r());
		}
		else if (m_joy1->has_pot_y())
		{
			data = m_joy1->pot_y_r();
		}
		else if (m_joy2->has_pot_y())
		{
			data = m_joy2->pot_y_r();
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia1_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::cia1_irq_w )
{
	m_cia1_irq = state;

	check_interrupts();
}

READ8_MEMBER( c128_state::cia1_pa_r )
{
	/*

	    bit     description

	    PA0     COL0, JOYB0
	    PA1     COL1, JOYB1
	    PA2     COL2, JOYB2
	    PA3     COL3, JOYB3
	    PA4     COL4, FBTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	UINT8 data = 0xff;

	// joystick
	UINT8 joy_b = m_joy2->joy_r();

	data &= (0xf0 | (joy_b & 0x0f));
	data &= ~(!BIT(joy_b, 5) << 4);

	// keyboard
	UINT8 cia1_pb = m_cia1->pb_r();
	UINT8 row[8] = { m_row0->read(), m_row1->read() & m_lock->read(), m_row2->read(), m_row3->read(),
						m_row4->read(), m_row5->read(), m_row6->read(), m_row7->read() };

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(cia1_pb, i))
		{
			if (!BIT(row[7], i)) data &= ~0x80;
			if (!BIT(row[6], i)) data &= ~0x40;
			if (!BIT(row[5], i)) data &= ~0x20;
			if (!BIT(row[4], i)) data &= ~0x10;
			if (!BIT(row[3], i)) data &= ~0x08;
			if (!BIT(row[2], i)) data &= ~0x04;
			if (!BIT(row[1], i)) data &= ~0x02;
			if (!BIT(row[0], i)) data &= ~0x01;
		}
	}

	return data;
}

WRITE8_MEMBER( c128_state::cia1_pa_w )
{
	/*

	    bit     description

	    PA0     COL0, JOYB0
	    PA1     COL1, JOYB1
	    PA2     COL2, JOYB2
	    PA3     COL3, JOYB3
	    PA4     COL4, FBTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	m_joy2->joy_w(data & 0x1f);
}

READ8_MEMBER( c128_state::cia1_pb_r )
{
	/*

	    bit     description

	    PB0     ROW0, JOYA0
	    PB1     ROW1, JOYA1
	    PB2     ROW2, JOYA2
	    PB3     ROW3, JOYA3
	    PB4     ROW4, FBTNA, _LP
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	UINT8 data = 0xff;

	// joystick
	UINT8 joy_a = m_joy1->joy_r();

	data &= (0xf0 | (joy_a & 0x0f));
	data &= ~(!BIT(joy_a, 5) << 4);

	// keyboard
	UINT8 cia1_pa = m_cia1->pa_r();

	if (!BIT(cia1_pa, 7)) data &= m_row7->read();
	if (!BIT(cia1_pa, 6)) data &= m_row6->read();
	if (!BIT(cia1_pa, 5)) data &= m_row5->read();
	if (!BIT(cia1_pa, 4)) data &= m_row4->read();
	if (!BIT(cia1_pa, 3)) data &= m_row3->read();
	if (!BIT(cia1_pa, 2)) data &= m_row2->read();
	if (!BIT(cia1_pa, 1)) data &= m_row1->read() & m_lock->read();
	if (!BIT(cia1_pa, 0)) data &= m_row0->read();

	if (!BIT(m_vic_k, 0)) data &= m_k0->read();
	if (!BIT(m_vic_k, 1)) data &= m_k1->read();
	if (!BIT(m_vic_k, 2)) data &= m_k2->read();

	return data;
}

WRITE8_MEMBER( c128_state::cia1_pb_w )
{
	/*

	    bit     description

	    PB0     ROW0, JOYA0
	    PB1     ROW1, JOYA1
	    PB2     ROW2, JOYA2
	    PB3     ROW3, JOYA3
	    PB4     ROW4, FBTNA, _LP
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	m_joy1->joy_w(data & 0x1f);

	m_vic->lp_w(BIT(data, 4));
}

WRITE_LINE_MEMBER( c128_state::cia1_cnt_w )
{
	m_cnt1 = state;
	m_user->write_4(state);

	update_iec();
}

WRITE_LINE_MEMBER( c128_state::cia1_sp_w )
{
	m_sp1 = state;
	m_user->write_5(state);

	update_iec();
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia2_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c128_state::cia2_irq_w )
{
	m_cia2_irq = state;

	check_interrupts();
}

READ8_MEMBER( c128_state::cia2_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     USER PORT
	    PA3
	    PA4
	    PA5
	    PA6     CLK
	    PA7     DATA

	*/

	UINT8 data = 0;

	// user port
	data |= m_user_pa2 << 2;

	// IEC bus
	data |= m_iec->clk_r() << 6;
	data |= m_iec->data_r() << 7;

	return data;
}

WRITE8_MEMBER( c128_state::cia2_pa_w )
{
	/*

	    bit     description

	    PA0     _VA14
	    PA1     _VA15
	    PA2     USER PORT
	    PA3     ATN OUT
	    PA4     CLK OUT
	    PA5     DATA OUT
	    PA6
	    PA7

	*/

	// VIC banking
	m_va14 = BIT(data, 0);
	m_va15 = BIT(data, 1);

	// user port
	m_user->write_m(BIT(data, 2));

	// IEC bus
	m_iec->atn_w(!BIT(data, 3));
	m_iec->clk_w(!BIT(data, 4));
	m_iec_data_out = BIT(data, 5);

	update_iec();
}

READ8_MEMBER( c128_state::cia2_pb_r )
{
	return m_user_pb;
}

WRITE8_MEMBER( c128_state::cia2_pb_w )
{
	m_user->write_c((data>>0)&1);
	m_user->write_d((data>>1)&1);
	m_user->write_e((data>>2)&1);
	m_user->write_f((data>>3)&1);
	m_user->write_h((data>>4)&1);
	m_user->write_j((data>>5)&1);
	m_user->write_k((data>>6)&1);
	m_user->write_l((data>>7)&1);
}

//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::cpu_r)
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4      CASS SENSE
	    P5
	    P6      CAPS LOCK

	*/

	UINT8 data = 0x07;

	// cassette sense
	data |= m_cassette->sense_r() << 4;

	// CAPS LOCK
	data |= m_caps_lock << 6;

	return data;
}

WRITE8_MEMBER( c128_state::cpu_w )
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3      CASS WRT
	    P4
	    P5      CASS MOTOR
	    P6

	*/

	// memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);

	// cassette write
	m_cassette->write(BIT(data, 3));

	// cassette motor
	m_cassette->motor_w(BIT(data, 5));
}


//-------------------------------------------------
//  CBM_IEC_INTERFACE( cbm_iec_intf )
//-------------------------------------------------

inline void c128_state::update_iec()
{
	int fsdir = m_mmu->fsdir_r();

	// fast serial data in
	int data_in = m_iec->data_r();

	m_cia1->sp_w(fsdir || data_in);

	// fast serial data out
	int data_out = !m_iec_data_out;

	if (fsdir) data_out &= m_sp1;

	m_iec->data_w(data_out);

	// fast serial clock in
	int srq_in = m_iec->srq_r();

	m_cia1->cnt_w(fsdir || srq_in);

	// fast serial clock out
	int srq_out = 1;

	if (fsdir) srq_out &= m_cnt1;

	m_iec->srq_w(srq_out);
}

WRITE_LINE_MEMBER( c128_state::iec_srq_w )
{
	update_iec();
}

WRITE_LINE_MEMBER( c128_state::iec_data_w )
{
	update_iec();
}


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

READ8_MEMBER( c128_state::exp_dma_cd_r )
{
	int ba = 0, aec = 1, z80io = 1;
	offs_t vma = 0;

	return read_memory(space, offset, vma, ba, aec, z80io);
}

WRITE8_MEMBER( c128_state::exp_dma_cd_w )
{
	int ba = 0, aec = 1, z80io = 1;
	offs_t vma = 0;

	return write_memory(space, offset, data, vma, ba, aec, z80io);
}

WRITE_LINE_MEMBER( c128_state::exp_irq_w )
{
	m_exp_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_nmi_w )
{
	m_exp_nmi = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_dma_w )
{
	m_exp_dma = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c128_state::exp_reset_w )
{
	if (!state)
	{
		machine_reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( c128dcr_iec_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( c128dcr_iec_devices )
	SLOT_INTERFACE("c1571", C1571)
	SLOT_INTERFACE("c1571cr", C1571CR)
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( c128d81_iec_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( c128d81_iec_devices )
	SLOT_INTERFACE("c1563", C1563)
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( c64 )
//-------------------------------------------------

void c128_state::machine_start()
{
	// allocate memory
	m_color_ram.allocate(0x800);

	// initialize memory
	UINT8 data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_z80en));
	save_item(NAME(m_loram));
	save_item(NAME(m_hiram));
	save_item(NAME(m_charen));
	save_item(NAME(m_game));
	save_item(NAME(m_exrom));
	save_item(NAME(m_reset));
	save_item(NAME(m_va14));
	save_item(NAME(m_va15));
	save_item(NAME(m_clrbank));
	save_item(NAME(m_cnt1));
	save_item(NAME(m_sp1));
	save_item(NAME(m_iec_data_out));
	save_item(NAME(m_cia1_irq));
	save_item(NAME(m_cia2_irq));
	save_item(NAME(m_vic_irq));
	save_item(NAME(m_exp_irq));
	save_item(NAME(m_exp_nmi));
	save_item(NAME(m_exp_dma));
	save_item(NAME(m_vic_k));
	save_item(NAME(m_caps_lock));
}


void c128_state::machine_reset()
{
	m_maincpu->reset();
	m_reset = 1;

	m_mmu->reset();
	m_vic->reset();
	m_vdc->reset();
	m_sid->reset();
	m_cia1->reset();
	m_cia2->reset();

	m_iec->reset();
	m_exp->reset();

	m_user->write_3(0);
	m_user->write_3(1);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc, c128_state )
	// basic hardware
	MCFG_CPU_ADD(Z80A_TAG, Z80, XTAL_14_31818MHz*2/3.5/2)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)
	MCFG_QUANTUM_PERFECT_CPU(Z80A_TAG)

	MCFG_CPU_ADD(M8502_TAG, M8502, XTAL_14_31818MHz*2/3.5/8)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_M8502_PORT_CALLBACKS(READ8(c128_state, cpu_r), WRITE8(c128_state, cpu_w))
	MCFG_M8502_PORT_PULLS(0x07, 0x20)
	MCFG_CPU_PROGRAM_MAP(m8502_mem)
	MCFG_QUANTUM_PERFECT_CPU(M8502_TAG)

	// video hardware
	MCFG_MOS8563_ADD(MOS8563_TAG, SCREEN_VDC_TAG, XTAL_16MHz, vdc_videoram_map)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_SCREEN_ADD(SCREEN_VDC_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS8563_TAG, mos8563_device, screen_update)

	MCFG_DEVICE_ADD(MOS8564_TAG, MOS8564, XTAL_14_31818MHz*2/3.5)
	MCFG_MOS6566_CPU(M8502_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(c128_state, vic_irq_w))
	MCFG_MOS8564_K_CALLBACK(WRITE8(c128_state, vic_k_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_VIC_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_VIC_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6567_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6567_COLUMNS, VIC6567_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS8564_TAG, mos8564_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", MOS8563_TAG":palette", c128)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_14_31818MHz*2/3.5/8)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c128_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c128_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_DEVICE_ADD(MOS8722_TAG, MOS8722, XTAL_14_31818MHz*2/3.5/8)
	MCFG_MOS8722_Z80EN_CALLBACK(WRITELINE(c128_state, mmu_z80en_w))
	MCFG_MOS8722_FSDIR_CALLBACK(WRITELINE(c128_state, mmu_fsdir_w))
	MCFG_MOS8722_GAME_CALLBACK(READLINE(c128_state, mmu_game_r))
	MCFG_MOS8722_EXROM_CALLBACK(READLINE(c128_state, mmu_exrom_r))
	MCFG_MOS8722_SENSE40_CALLBACK(READLINE(c128_state, mmu_sense40_r))
	MCFG_MOS8721_ADD(MOS8721_TAG)
	MCFG_DEVICE_ADD(MOS6526_1_TAG, MOS6526, XTAL_14_31818MHz*2/3.5/8)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c128_state, cia1_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(WRITELINE(c128_state, cia1_cnt_w))
	MCFG_MOS6526_SP_CALLBACK(WRITELINE(c128_state, cia1_sp_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c128_state, cia1_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c128_state, cia1_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c128_state, cia1_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c128_state, cia1_pb_w))
	MCFG_DEVICE_ADD(MOS6526_2_TAG, MOS6526, XTAL_14_31818MHz*2/3.5/8)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c128_state, cia2_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_6))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_7))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c128_state, cia2_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c128_state, cia2_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c128_state, cia2_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c128_state, cia2_pb_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_8))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c1530", DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS8564_TAG, mos8564_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, "joy")
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, XTAL_14_31818MHz*2/3.5/8, c64_expansion_cards, nullptr)
	MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(WRITELINE(c128_state, exp_irq_w))
	MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(WRITELINE(c128_state, exp_nmi_w))
	MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(WRITELINE(c128_state, exp_reset_w))
	MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(READ8(c128_state, exp_dma_cd_r))
	MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(WRITE8(c128_state, exp_dma_cd_w))
	MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(WRITELINE(c128_state, exp_dma_w))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, c64_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_3_HANDLER(WRITELINE(c128_state, exp_reset_w))
	MCFG_PET_USER_PORT_4_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_5_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_6_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_7_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_9_HANDLER(DEVWRITELINE(CBM_IEC_TAG, cbm_iec_device, atn_w))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_PET_USER_PORT_C_HANDLER(WRITELINE(c128_state, write_user_pb0))
	MCFG_PET_USER_PORT_D_HANDLER(WRITELINE(c128_state, write_user_pb1))
	MCFG_PET_USER_PORT_E_HANDLER(WRITELINE(c128_state, write_user_pb2))
	MCFG_PET_USER_PORT_F_HANDLER(WRITELINE(c128_state, write_user_pb3))
	MCFG_PET_USER_PORT_H_HANDLER(WRITELINE(c128_state, write_user_pb4))
	MCFG_PET_USER_PORT_J_HANDLER(WRITELINE(c128_state, write_user_pb5))
	MCFG_PET_USER_PORT_K_HANDLER(WRITELINE(c128_state, write_user_pb6))
	MCFG_PET_USER_PORT_L_HANDLER(WRITELINE(c128_state, write_user_pb7))
	MCFG_PET_USER_PORT_M_HANDLER(WRITELINE(c128_state, write_user_pa2))

	MCFG_QUICKLOAD_ADD("quickload", c128_state, cbm_c64, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "c128_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list_c64", "c64_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list_c64", "c64_flop")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "c128_flop")
	MCFG_SOFTWARE_LIST_ADD("from_list", "c128_rom")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("cass_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("flop_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("from_list", "NTSC")

	// function ROM
	MCFG_GENERIC_SOCKET_ADD("from", generic_plain_slot, "c128_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128, ntsc )
	MCFG_CBM_IEC_ADD("c1571")
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(WRITELINE(c128_state, iec_srq_w))
	MCFG_CBM_IEC_BUS_DATA_CALLBACK(WRITELINE(c128_state, iec_data_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dcr )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dcr, ntsc )
	MCFG_CBM_IEC_ADD("c1571") // TODO c1571cr
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(WRITELINE(c128_state, iec_srq_w))
	MCFG_CBM_IEC_BUS_DATA_CALLBACK(WRITELINE(c128_state, iec_data_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128d81 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128d81, ntsc )
	MCFG_CBM_IEC_ADD(nullptr)
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(WRITELINE(c128_state, iec_srq_w))
	MCFG_CBM_IEC_BUS_DATA_CALLBACK(WRITELINE(c128_state, iec_data_w))

	MCFG_DEVICE_MODIFY("iec8")
	MCFG_DEVICE_SLOT_INTERFACE(c128d81_iec_devices, "c1563", false)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal, c128_state )
	// basic hardware
	MCFG_CPU_ADD(Z80A_TAG, Z80, XTAL_17_734472MHz*2/4.5/2)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)
	MCFG_QUANTUM_PERFECT_CPU(Z80A_TAG)

	MCFG_CPU_ADD(M8502_TAG, M8502, XTAL_17_734472MHz*2/4.5/8)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_M8502_PORT_CALLBACKS(READ8(c128_state, cpu_r), WRITE8(c128_state, cpu_w))
	MCFG_M8502_PORT_PULLS(0x07, 0x20)
	MCFG_CPU_PROGRAM_MAP(m8502_mem)
	MCFG_QUANTUM_PERFECT_CPU(M8502_TAG)

	// video hardware
	MCFG_MOS8563_ADD(MOS8563_TAG, SCREEN_VDC_TAG, XTAL_16MHz, vdc_videoram_map)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_SCREEN_ADD(SCREEN_VDC_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS8563_TAG, mos8563_device, screen_update)

	MCFG_DEVICE_ADD(MOS8566_TAG, MOS8566, XTAL_17_734472MHz*2/4.5)
	MCFG_MOS6566_CPU(M8502_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(c128_state, vic_irq_w))
	MCFG_MOS8564_K_CALLBACK(WRITE8(c128_state, vic_k_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_VIC_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_VIC_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6569_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6569_COLUMNS, VIC6569_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS8566_TAG, mos8566_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", MOS8563_TAG":palette", c128)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_17_734472MHz*2/4.5/8)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c128_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c128_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_DEVICE_ADD(MOS8722_TAG, MOS8722, XTAL_17_734472MHz*2/4.5/8)
	MCFG_MOS8722_Z80EN_CALLBACK(WRITELINE(c128_state, mmu_z80en_w))
	MCFG_MOS8722_FSDIR_CALLBACK(WRITELINE(c128_state, mmu_fsdir_w))
	MCFG_MOS8722_GAME_CALLBACK(READLINE(c128_state, mmu_game_r))
	MCFG_MOS8722_EXROM_CALLBACK(READLINE(c128_state, mmu_exrom_r))
	MCFG_MOS8722_SENSE40_CALLBACK(READLINE(c128_state, mmu_sense40_r))
	MCFG_MOS8721_ADD(MOS8721_TAG)
	MCFG_DEVICE_ADD(MOS6526_1_TAG, MOS6526, XTAL_17_734472MHz*2/4.5/8)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c128_state, cia1_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(WRITELINE(c128_state, cia1_cnt_w))
	MCFG_MOS6526_SP_CALLBACK(WRITELINE(c128_state, cia1_sp_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c128_state, cia1_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c128_state, cia1_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c128_state, cia1_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c128_state, cia1_pb_w))
	MCFG_DEVICE_ADD(MOS6526_2_TAG, MOS6526, XTAL_17_734472MHz*2/4.5/8)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c128_state, cia2_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_6))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_7))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c128_state, cia2_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c128_state, cia2_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c128_state, cia2_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c128_state, cia2_pb_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_8))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c1530", DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS8566_TAG, mos8566_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, "joy")
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, XTAL_17_734472MHz*2/4.5/8, c64_expansion_cards, nullptr)
	MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(WRITELINE(c128_state, exp_irq_w))
	MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(WRITELINE(c128_state, exp_nmi_w))
	MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(WRITELINE(c128_state, exp_reset_w))
	MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(READ8(c128_state, exp_dma_cd_r))
	MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(WRITE8(c128_state, exp_dma_cd_w))
	MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(WRITELINE(c128_state, exp_dma_w))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, c64_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_3_HANDLER(WRITELINE(c128_state, exp_reset_w))
	MCFG_PET_USER_PORT_4_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_5_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_6_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_7_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_9_HANDLER(DEVWRITELINE(CBM_IEC_TAG, cbm_iec_device, atn_w))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_PET_USER_PORT_C_HANDLER(WRITELINE(c128_state, write_user_pb0))
	MCFG_PET_USER_PORT_D_HANDLER(WRITELINE(c128_state, write_user_pb1))
	MCFG_PET_USER_PORT_E_HANDLER(WRITELINE(c128_state, write_user_pb2))
	MCFG_PET_USER_PORT_F_HANDLER(WRITELINE(c128_state, write_user_pb3))
	MCFG_PET_USER_PORT_H_HANDLER(WRITELINE(c128_state, write_user_pb4))
	MCFG_PET_USER_PORT_J_HANDLER(WRITELINE(c128_state, write_user_pb5))
	MCFG_PET_USER_PORT_K_HANDLER(WRITELINE(c128_state, write_user_pb6))
	MCFG_PET_USER_PORT_L_HANDLER(WRITELINE(c128_state, write_user_pb7))
	MCFG_PET_USER_PORT_M_HANDLER(WRITELINE(c128_state, write_user_pa2))

	MCFG_QUICKLOAD_ADD("quickload", c128_state, cbm_c64, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "c128_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list_c64", "c64_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list_c64", "c64_flop")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "c128_flop")
	MCFG_SOFTWARE_LIST_ADD("from_list", "c128_rom")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cass_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("flop_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("from_list", "PAL")

	// function ROM
	MCFG_GENERIC_SOCKET_ADD("from", generic_plain_slot, "c128_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128pal, pal )
	MCFG_CBM_IEC_ADD("c1571")
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(WRITELINE(c128_state, iec_srq_w))
	MCFG_CBM_IEC_BUS_DATA_CALLBACK(WRITELINE(c128_state, iec_data_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( c128dcrp )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( c128dcrp, pal )
	MCFG_CBM_IEC_ADD("c1571") // TODO c1571cr
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(WRITELINE(c128_state, iec_srq_w))
	MCFG_CBM_IEC_BUS_DATA_CALLBACK(WRITELINE(c128_state, iec_data_w))
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c128 )
//-------------------------------------------------

ROM_START( c128 )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_DEFAULT_BIOS("r4")
	ROM_LOAD( "251913-01.u32", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )
	ROM_SYSTEM_BIOS( 0, "r2", "Revision 2" )
	ROMX_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(1) )
	ROMX_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(1) )
	ROMX_LOAD( "318020-03.u35", 0xc000, 0x4000, CRC(1e94bb02) SHA1(e80ffbafae068cc0e42698ec5c5c39af46ac612a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(2) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(2) )
	ROMX_LOAD( "318020-05.u35", 0xc000, 0x4000, CRC(ba456b8e) SHA1(ceb6e1a1bf7e08eb9cbc651afa29e26adccf38ab), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(3) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(3) )
	ROMX_LOAD( "jiffydos c128.u35", 0xc000, 0x4000, CRC(4b7964de) SHA1(7d1898f32beae4b2ae610d469ce578a588efaa7c), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "quikslvr", "QuickSilver 128" ) // requires add-on cartridge
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(4) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(4) )
	ROMX_LOAD( "quicksilver128.u35", 0xc000, 0x4000, CRC(c2e74338) SHA1(916cdcc62eb631073aa7f096815dcf33b3229ca8), ROM_BIOS(4) )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END

#define rom_c128p       rom_c128
#define rom_c128d       rom_c128
#define rom_c128dp      rom_c128
#define rom_c128dpr     rom_c128
#define rom_c128d81     rom_c128


//-------------------------------------------------
//  ROM( c128_de )
//-------------------------------------------------

ROM_START( c128_de )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_DEFAULT_BIOS("r4")
	ROM_LOAD( "251913-01.u32", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )
	ROM_SYSTEM_BIOS( 0, "r2", "Revision 2" )
	ROMX_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6), ROM_BIOS(1) )
	ROMX_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f), ROM_BIOS(1) )
	ROMX_LOAD( "315078-01.u35", 0xc000, 0x4000, CRC(a51e2168) SHA1(bcf82a89a8fc5d086bec2ff3bcbdecc8af2be3af), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318018-04.u33", 0x4000, 0x4000, CRC(9f9c355b) SHA1(d53a7884404f7d18ebd60dd3080c8f8d71067441), ROM_BIOS(2) )
	ROMX_LOAD( "318019-04.u34", 0x8000, 0x4000, CRC(6e2c91a7) SHA1(c4fb4a714e48a7bf6c28659de0302183a0e0d6c0), ROM_BIOS(2) )
	ROMX_LOAD( "315078-02.u35", 0xc000, 0x4000, CRC(b275bb2e) SHA1(78ac5dcdd840b092ba1ee6d19b33af079613291f), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "315079-01.u18", 0x00000, 0x2000, CRC(fe5a2db1) SHA1(638f8aff51c2ac4f99a55b12c4f8c985ef4bebd3) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128_se )
//-------------------------------------------------

ROM_START( c128_se )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "325182-01.u32", 0x0000, 0x4000, CRC(2aff27d3) SHA1(267654823c4fdf2167050f41faa118218d2569ce) ) // "C128 64 Sw/Fi"
	ROM_LOAD( "318018-02.u33", 0x4000, 0x4000, CRC(2ee6e2fa) SHA1(60e1491e1d5782e3cf109f518eb73427609badc6) )
	ROM_LOAD( "318019-02.u34", 0x8000, 0x4000, CRC(d551fce0) SHA1(4d223883e866645328f86a904b221464682edc4f) )
	ROM_LOAD( "325189-01.u35", 0xc000, 0x4000, CRC(9526fac4) SHA1(a01dd871241c801db51e8ebc30fedfafd8cc506b) ) // "C128 Ker Sw/Fi"

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "325181-01.bin", 0x0000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) ) // "C128 Char Sw/Fi"

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128cr )
//-------------------------------------------------

ROM_START( c128cr )
	/* C128CR prototype, owned by Bo Zimmers
	PCB markings: "COMMODORE 128CR REV.3 // PCB NO.252270" and "PCB ASSY NO.250783"
	Sticker on rom cart shield: "C128CR  No.2 // ENG. SAMPLE // Jun/9/'86   KNT"
	3 ROMs (combined basic, combined c64/kernal, plain character rom)
	6526A-1 CIAs
	?prototype? 2568R1X VDC w/ 1186 datecode
	*/
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "252343-03.u34", 0x4000, 0x8000, CRC(bc07ed87) SHA1(0eec437994a3f2212343a712847213a8a39f4a7b) ) // "252343-03 // U34"
	ROM_LOAD( "252343-04.u32", 0x0000, 0x4000, CRC(cc6bdb69) SHA1(36286b2e8bea79f7767639fd85e12c5447c7041b) ) // "252343-04 // US // U32"
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) ) // "MOS // (C)1985 CBM // 390059-01 // M468613 8547H"

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128dcr )
//-------------------------------------------------

ROM_START( c128dcr )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318023-02.u32", 0x0000, 0x4000, CRC(eedc120a) SHA1(f98c5a986b532c78bb68df9ec6dbcf876913b99f) )
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "390059-01.u18", 0x0000, 0x2000, CRC(6aaaafe6) SHA1(29ed066d513f2d5c09ff26d9166ba23c2afb2b3f) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END

#define rom_c128dcrp    rom_c128dcr


//-------------------------------------------------
//  ROM( c128dcr_de )
//-------------------------------------------------

ROM_START( c128dcr_de )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318077-01.u32", 0x0000, 0x4000, CRC(eb6e2c8f) SHA1(6b3d891fedabb5335f388a5d2a71378472ea60f4) )
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "315079-01.u18", 0x0000, 0x2000, CRC(fe5a2db1) SHA1(638f8aff51c2ac4f99a55b12c4f8c985ef4bebd3) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END


//-------------------------------------------------
//  ROM( c128dcr_se )
//-------------------------------------------------

ROM_START( c128dcr_se )
	ROM_REGION( 0x10000, M8502_TAG, 0 )
	ROM_LOAD( "318022-02.u34", 0x4000, 0x8000, CRC(af1ae1e8) SHA1(953dcdf5784a6b39ef84dd6fd968c7a03d8d6816) )
	ROM_LOAD( "318034-01.u32", 0x0000, 0x4000, CRC(cb4e1719) SHA1(9b0a0cef56d00035c611e07170f051ee5e63aa3a) )
	ROM_CONTINUE(              0xc000, 0x4000 )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "325181-01.u18", 0x0000, 0x2000, CRC(7a70d9b8) SHA1(aca3f7321ee7e6152f1f0afad646ae41964de4fb) )

	ROM_REGION( 0xc88, MOS8721_TAG, 0 )
	// converted from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c128/8721-reduced.zip/8721-reduced.txt
	ROM_LOAD( "8721r3.u11", 0x000, 0xc88, BAD_DUMP CRC(154db186) SHA1(ccadcdb1db3b62c51dc4ce60fe6f96831586d297) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT                COMPANY                        FULLNAME                                 FLAGS
COMP( 1985, c128,       0,      0,      c128,       c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128 (NTSC)",                  MACHINE_SUPPORTS_SAVE )
COMP( 1985, c128p,      0,      0,      c128pal,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128 (PAL)",                   MACHINE_SUPPORTS_SAVE )
COMP( 1985, c128_de,    c128,   0,      c128pal,    c128_de,    driver_device,  0,  "Commodore Business Machines", "Commodore 128 (Germany)",               MACHINE_SUPPORTS_SAVE )
//COMP( 1985, c128_fr,   c128,  0,   c128pal,  c128_fr, driver_device, 0,  "Commodore Business Machines", "Commodore 128 (France)", MACHINE_SUPPORTS_SAVE )
//COMP( 1985, c128_no,   c128,  0,   c128pal,  c128_it, driver_device, 0,  "Commodore Business Machines", "Commodore 128 (Norway)", MACHINE_SUPPORTS_SAVE )
COMP( 1985, c128_se,    c128,   0,      c128pal,    c128_se,    driver_device,  0,  "Commodore Business Machines", "Commodore 128 (Sweden/Finland)",        MACHINE_SUPPORTS_SAVE )
COMP( 1986, c128d,      c128,   0,      c128,       c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128D (NTSC, prototype)",      MACHINE_SUPPORTS_SAVE )
COMP( 1986, c128dp,     c128,   0,      c128pal,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128D (PAL)",                  MACHINE_SUPPORTS_SAVE )

COMP( 1986, c128cr,     c128,   0,      c128,       c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128CR (NTSC, prototype)",     MACHINE_SUPPORTS_SAVE )

COMP( 1987, c128dcr,    c128,   0,      c128dcr,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (NTSC)",               MACHINE_SUPPORTS_SAVE )
COMP( 1987, c128dcrp,   c128,   0,      c128dcrp,   c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (PAL)",                MACHINE_SUPPORTS_SAVE )
COMP( 1987, c128dcr_de, c128,   0,      c128dcrp,   c128_de,    driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (Germany)",            MACHINE_SUPPORTS_SAVE )
//COMP( 1986, c128dcr_it,  c128,  0,   c128dcrp, c128_it, driver_device, 0,"Commodore Business Machines", "Commodore 128DCR (Italy)", MACHINE_SUPPORTS_SAVE )
COMP( 1987, c128dcr_se, c128,   0,      c128dcrp,   c128_se,    driver_device,  0,  "Commodore Business Machines", "Commodore 128DCR (Sweden/Finland)",     MACHINE_SUPPORTS_SAVE )

COMP( 1986, c128d81,    c128,   0,      c128d81,    c128,       driver_device,  0,  "Commodore Business Machines", "Commodore 128D/81 (NTSC, prototype)",   MACHINE_SUPPORTS_SAVE )
