// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - floating bus writes to peripheral registers in m6502.c
    - sort out kernals between PAL/NTSC
    - tsuit215 test failures
        - IRQ (WRONG $DC0D)
        - NMI (WRONG $DD0D)
        - some CIA tests
    - PDC Clipper (C64 in a briefcase with 3" floppy, electroluminescent flat screen, thermal printer)

*/

#include "includes/c64.h"
#include "bus/cbmiec/c1541.h"
#include "machine/cbm_snqk.h"
#include "softlist.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define VA13 BIT(va, 13)
#define VA12 BIT(va, 12)

enum
{
	PLA_OUT_CASRAM = 0,
	PLA_OUT_BASIC  = 1,
	PLA_OUT_KERNAL = 2,
	PLA_OUT_CHAROM = 3,
	PLA_OUT_GRW    = 4,
	PLA_OUT_IO     = 5,
	PLA_OUT_ROML   = 6,
	PLA_OUT_ROMH   = 7
};


QUICKLOAD_LOAD_MEMBER( c64_state, cbm_c64 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void c64_state::check_interrupts()
{
	int irq = m_cia1_irq || m_vic_irq || m_exp_irq;
	int nmi = m_cia2_irq || !m_restore || m_exp_nmi;
	//int rdy = m_exp_dma && m_vic_ba;

	m_maincpu->set_input_line(M6510_IRQ_LINE, irq);
	m_maincpu->set_input_line(M6510_NMI_LINE, nmi);
}



//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla -
//-------------------------------------------------

int c64_state::read_pla(offs_t offset, offs_t va, int rw, int aec, int ba)
{
	//int ba = m_vic->ba_r();
	//int aec = !m_vic->aec_r();
	int sphi2 = m_vic->phi0_r();
	int game = m_exp->game_r(offset, sphi2, ba, rw, m_hiram);
	int exrom = m_exp->exrom_r(offset, sphi2, ba, rw, m_hiram);
	int cas = 0;

	UINT32 input = VA12 << 15 | VA13 << 14 | game << 13 | exrom << 12 | rw << 11 | aec << 10 | ba << 9 | A12 << 8 |
		A13 << 7 | A14 << 6 | A15 << 5 | m_va14 << 4 | m_charen << 3 | m_hiram << 2 | m_loram << 1 | cas;

	return m_pla->read(input);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 c64_state::read_memory(address_space &space, offs_t offset, offs_t va, int aec, int ba)
{
	int rw = 1;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	int plaout = read_pla(offset, va, rw, !aec, ba);

	UINT8 data = 0xff;

	if (!aec)
	{
		data = m_vic->bus_r();
	}

	if (!BIT(plaout, PLA_OUT_CASRAM))
	{
		if (aec)
		{
			data = m_ram->pointer()[offset];
		}
		else
		{
			data = m_ram->pointer()[(!m_va15 << 15) | (!m_va14 << 14) | va];
		}
	}
	if (!BIT(plaout, PLA_OUT_BASIC))
	{
		data = m_basic[offset & 0x1fff];
	}
	if (!BIT(plaout, PLA_OUT_KERNAL))
	{
		data = m_kernal[offset & 0x1fff];
	}
	if (!BIT(plaout, PLA_OUT_CHAROM))
	{
		data = m_charom[offset & 0xfff];
	}
	if (!BIT(plaout, PLA_OUT_IO))
	{
		switch ((offset >> 8) & 0x0f)
		{
		case 0:
		case 1:
		case 2:
		case 3: // VIC
			data = m_vic->read(space, offset & 0x3f);
			break;

		case 4:
		case 5:
		case 6:
		case 7: // SID
			data = m_sid->read(space, offset & 0x1f);
			break;

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb: // COLOR
			data = m_color_ram[offset & 0x3ff] & 0x0f;
			break;

		case 0xc: // CIA1
			data = m_cia1->read(space, offset & 0x0f);
			break;

		case 0xd: // CIA2
			data = m_cia2->read(space, offset & 0x0f);
			break;

		case 0xe: // I/O1
			io1 = 0;
			break;

		case 0xf: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);
	return m_exp->cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void c64_state::write_memory(address_space &space, offs_t offset, UINT8 data, int aec, int ba)
{
	int rw = 0;
	offs_t va = 0;
	int io1 = 1, io2 = 1;
	int sphi2 = m_vic->phi0_r();

	int plaout = read_pla(offset, va, rw, !aec, ba);

	if (offset < 0x0002)
	{
		// write to internal CPU register
		data = m_vic->bus_r();
	}

	if (!BIT(plaout, PLA_OUT_CASRAM))
	{
		m_ram->pointer()[offset] = data;
	}
	if (!BIT(plaout, PLA_OUT_IO))
	{
		switch ((offset >> 8) & 0x0f)
		{
		case 0:
		case 1:
		case 2:
		case 3: // VIC
			m_vic->write(space, offset & 0x3f, data);
			break;

		case 4:
		case 5:
		case 6:
		case 7: // SID
			m_sid->write(space, offset & 0x1f, data);
			break;

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb: // COLOR
			if (!BIT(plaout, PLA_OUT_GRW)) m_color_ram[offset & 0x3ff] = data & 0x0f;
			break;

		case 0xc: // CIA1
			m_cia1->write(space, offset & 0x0f, data);
			break;

		case 0xd: // CIA2
			m_cia2->write(space, offset & 0x0f, data);
			break;

		case 0xe: // I/O1
			io1 = 0;
			break;

		case 0xf: // I/O2
			io2 = 0;
			break;
		}
	}

	int roml = BIT(plaout, PLA_OUT_ROML);
	int romh = BIT(plaout, PLA_OUT_ROMH);
	m_exp->cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( c64_state::read )
{
	int aec = 1, ba = 1;

	// VIC address bus is floating
	offs_t va = 0x3fff;

	return read_memory(space, offset, va, aec, ba);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( c64_state::write )
{
	int aec = 1, ba = 1;

	write_memory(space, offset, data, aec, ba);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( c64_state::vic_videoram_r )
{
	int aec = m_vic->aec_r(), ba = m_vic->ba_r();
	offs_t va = offset;

	// A15/A14 are not connected to VIC so they are floating
	//offset |= 0xc000;

	return read_memory(space, offset, va, aec, ba);
}


//-------------------------------------------------
//  vic_colorram_r -
//-------------------------------------------------

READ8_MEMBER( c64_state::vic_colorram_r )
{
	UINT8 data;

	if (m_vic->aec_r())
	{
		// TODO low nibble of last opcode
		data = 0x0f;
	}
	else
	{
		data = m_color_ram[offset] & 0x0f;
	}

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( c64_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c64_mem, AS_PROGRAM, 8, c64_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, c64_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, c64_state )
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_colorram_r)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c64 )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::write_restore )
{
	m_restore = state;

	check_interrupts();
}

static INPUT_PORTS_START( c64 )
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

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_state, write_restore)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c64sw )
//-------------------------------------------------

static INPUT_PORTS_START( c64sw )
	PORT_INCLUDE( c64 )

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR(':') PORT_CHAR('*')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c64gs )
//-------------------------------------------------

static INPUT_PORTS_START( c64gs )
	// no keyboard
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  vic2_interface vic_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::vic_irq_w )
{
	m_vic_irq = state;

	check_interrupts();
}


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

READ8_MEMBER( c64_state::sid_potx_r )
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

READ8_MEMBER( c64_state::sid_poty_r )
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

WRITE_LINE_MEMBER( c64_state::cia1_irq_w )
{
	m_cia1_irq = state;

	check_interrupts();
}

READ8_MEMBER( c64_state::cia1_pa_r )
{
	/*

	    bit     description

	    PA0     COL0, JOY B0
	    PA1     COL1, JOY B1
	    PA2     COL2, JOY B2
	    PA3     COL3, JOY B3
	    PA4     COL4, BTNB
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

WRITE8_MEMBER( c64_state::cia1_pa_w )
{
	/*

	    bit     description

	    PA0     COL0, JOY B0
	    PA1     COL1, JOY B1
	    PA2     COL2, JOY B2
	    PA3     COL3, JOY B3
	    PA4     COL4, BTNB
	    PA5     COL5
	    PA6     COL6
	    PA7     COL7

	*/

	m_joy2->joy_w(data & 0x1f);
}

READ8_MEMBER( c64_state::cia1_pb_r )
{
	/*

	    bit     description

	    PB0     ROW0, JOY A0
	    PB1     ROW1, JOY A1
	    PB2     ROW2, JOY A2
	    PB3     ROW3, JOY A3
	    PB4     ROW4, BTNA, _LP
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

	return data;
}

WRITE8_MEMBER( c64_state::cia1_pb_w )
{
	/*

	    bit     description

	    PB0     ROW0, JOY A0
	    PB1     ROW1, JOY A1
	    PB2     ROW2, JOY A2
	    PB3     ROW3, JOY A3
	    PB4     ROW4, BTNA, _LP
	    PB5     ROW5
	    PB6     ROW6
	    PB7     ROW7

	*/

	m_joy1->joy_w(data & 0x1f);

	m_vic->lp_w(BIT(data, 4));
}

READ8_MEMBER( c64gs_state::cia1_pa_r )
{
	/*

	    bit     description

	    PA0     JOY B0
	    PA1     JOY B1
	    PA2     JOY B2
	    PA3     JOY B3
	    PA4     BTNB
	    PA5
	    PA6
	    PA7

	*/

	UINT8 data = 0xff;

	// joystick
	UINT8 joy_b = m_joy2->joy_r();

	data &= (0xf0 | (joy_b & 0x0f));
	data &= ~(!BIT(joy_b, 5) << 4);

	return data;
}

READ8_MEMBER( c64gs_state::cia1_pb_r )
{
	/*

	    bit     description

	    PB0     JOY A0
	    PB1     JOY A1
	    PB2     JOY A2
	    PB3     JOY A3
	    PB4     BTNA/_LP
	    PB5
	    PB6
	    PB7

	*/

	UINT8 data = 0xff;

	// joystick
	UINT8 joy_a = m_joy1->joy_r();

	data &= (0xf0 | (joy_a & 0x0f));
	data &= ~(!BIT(joy_a, 5) << 4);

	return data;
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia2_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::cia2_irq_w )
{
	m_cia2_irq = state;

	check_interrupts();
}

READ8_MEMBER( c64_state::cia2_pa_r )
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

WRITE8_MEMBER( c64_state::cia2_pa_w )
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
	m_iec->data_w(!BIT(data, 5));
}

READ8_MEMBER( c64_state::cia2_pb_r )
{
	return m_user_pb;
}

WRITE8_MEMBER( c64_state::cia2_pb_w )
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

READ8_MEMBER( c64_state::cpu_r )
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4      CASS SENS
	    P5      0

	*/

	UINT8 data = 0x07;

	data |= m_cassette->sense_r() << 4;

	return data;
}

WRITE8_MEMBER( c64_state::cpu_w )
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3      CASS WRT
	    P4
	    P5      CASS MOTOR

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
//  M6510_INTERFACE( sx64_cpu_intf )
//-------------------------------------------------

READ8_MEMBER( sx64_state::cpu_r )
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4
	    P5

	*/

	return 0x07;
}

WRITE8_MEMBER( sx64_state::cpu_w )
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3
	    P4
	    P5

	*/

	// memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);
}


//-------------------------------------------------
//  M6510_INTERFACE( c64gs_cpu_intf )
//-------------------------------------------------

READ8_MEMBER( c64gs_state::cpu_r )
{
	/*

	    bit     description

	    P0      1
	    P1      1
	    P2      1
	    P3
	    P4
	    P5

	*/

	return 0x07;
}

WRITE8_MEMBER( c64gs_state::cpu_w )
{
	/*

	    bit     description

	    P0      LORAM
	    P1      HIRAM
	    P2      CHAREN
	    P3
	    P4
	    P5

	*/

	// memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);
}


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::exp_irq_w )
{
	m_exp_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c64_state::exp_nmi_w )
{
	m_exp_nmi = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c64_state::exp_dma_w )
{
	if (m_exp_dma != state)
	{
		m_exp_dma = state;

		m_maincpu->set_input_line(INPUT_LINE_HALT, m_exp_dma);
	}
}

WRITE_LINE_MEMBER( c64_state::exp_reset_w )
{
	if (!state)
	{
		machine_reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( sx1541_iec_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( sx1541_iec_devices )
	SLOT_INTERFACE("sx1541", SX1541)
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( c64 )
//-------------------------------------------------

void c64_state::machine_start()
{
	// get pointers to ROMs
	if (memregion("basic") != nullptr)
	{
		m_basic = memregion("basic")->base();
		m_kernal = memregion("kernal")->base();
	}
	else
	{
		m_basic = memregion("kernal")->base();
		m_kernal = &m_basic[0x2000];
	}
	m_charom = memregion("charom")->base();

	// allocate memory
	m_color_ram.allocate(0x400);

	// initialize memory
	UINT8 data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_loram));
	save_item(NAME(m_hiram));
	save_item(NAME(m_charen));
	save_item(NAME(m_va14));
	save_item(NAME(m_va15));
	save_item(NAME(m_cia1_irq));
	save_item(NAME(m_cia2_irq));
	save_item(NAME(m_vic_irq));
	save_item(NAME(m_exp_irq));
	save_item(NAME(m_exp_nmi));
	save_item(NAME(m_exp_dma));
	save_item(NAME(m_user_pb));
	save_item(NAME(m_user_pa2));
}


void c64_state::machine_reset()
{
	m_maincpu->reset();

	m_vic->reset();
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

static MACHINE_CONFIG_START( ntsc, c64_state )
	// basic hardware
	MCFG_CPU_ADD(M6510_TAG, M6510, XTAL_14_31818MHz/14)
	MCFG_CPU_PROGRAM_MAP(c64_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_M6510_PORT_CALLBACKS(READ8(c64_state, cpu_r), WRITE8(c64_state, cpu_w))
	MCFG_M6510_PORT_PULLS(0x17, 0xc8)
	MCFG_QUANTUM_PERFECT_CPU(M6510_TAG)

	// video hardware
	MCFG_DEVICE_ADD(MOS6567_TAG, MOS6567, XTAL_14_31818MHz/14)
	MCFG_MOS6566_CPU(M6510_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(c64_state, vic_irq_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6567_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6567_COLUMNS, VIC6567_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS6567_TAG, mos6567_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_14_31818MHz/14)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c64_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c64_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_DEVICE_ADD(MOS6526_1_TAG, MOS6526, XTAL_14_31818MHz/14)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c64_state, cia1_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_4))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_5))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c64_state, cia1_pa_r))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c64_state, cia1_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c64_state, cia1_pb_w))
	MCFG_DEVICE_ADD(MOS6526_2_TAG, MOS6526, XTAL_14_31818MHz/14)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c64_state, cia2_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_6))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_7))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c64_state, cia2_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c64_state, cia2_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c64_state, cia2_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c64_state, cia2_pb_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_8))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c1530", DEVWRITELINE(MOS6526_1_TAG, mos6526_device, flag_w))
	MCFG_CBM_IEC_ADD("c1541")
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, flag_w))
	MCFG_CBM_IEC_BUS_ATN_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_9))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS6567_TAG, mos6567_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, "joy")
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, XTAL_14_31818MHz/14, c64_expansion_cards, nullptr)
	MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(WRITELINE(c64_state, exp_irq_w))
	MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(WRITELINE(c64_state, exp_nmi_w))
	MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(WRITELINE(c64_state, exp_reset_w))
	MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(READ8(c64_state, read))
	MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(WRITE8(c64_state, write))
	MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(WRITELINE(c64_state, exp_dma_w))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, c64_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_3_HANDLER(WRITELINE(c64_state, exp_reset_w))
	MCFG_PET_USER_PORT_4_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_5_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_6_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_7_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_9_HANDLER(DEVWRITELINE(CBM_IEC_TAG, cbm_iec_device, atn_w))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_PET_USER_PORT_C_HANDLER(WRITELINE(c64_state, write_user_pb0))
	MCFG_PET_USER_PORT_D_HANDLER(WRITELINE(c64_state, write_user_pb1))
	MCFG_PET_USER_PORT_E_HANDLER(WRITELINE(c64_state, write_user_pb2))
	MCFG_PET_USER_PORT_F_HANDLER(WRITELINE(c64_state, write_user_pb3))
	MCFG_PET_USER_PORT_H_HANDLER(WRITELINE(c64_state, write_user_pb4))
	MCFG_PET_USER_PORT_J_HANDLER(WRITELINE(c64_state, write_user_pb5))
	MCFG_PET_USER_PORT_K_HANDLER(WRITELINE(c64_state, write_user_pb6))
	MCFG_PET_USER_PORT_L_HANDLER(WRITELINE(c64_state, write_user_pb7))
	MCFG_PET_USER_PORT_M_HANDLER(WRITELINE(c64_state, write_user_pa2))

	MCFG_QUICKLOAD_ADD("quickload", c64_state, cbm_c64, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "c64_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "c64_flop")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("cass_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "NTSC")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet64 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet64, ntsc )
	// TODO monochrome green palette
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc_sx )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc_sx, sx64_state )
	MCFG_FRAGMENT_ADD(ntsc)

	// basic hardware
	MCFG_CPU_MODIFY(M6510_TAG)
	MCFG_M6510_PORT_CALLBACKS(READ8(sx64_state, cpu_r), WRITE8(sx64_state, cpu_w))
	MCFG_M6510_PORT_PULLS(0x07, 0xc0)

	// devices
	MCFG_DEVICE_MODIFY("iec8")
	MCFG_DEVICE_SLOT_INTERFACE(sx1541_iec_devices, "sx1541", false)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc_dx )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc_dx, sx64_state )
	MCFG_FRAGMENT_ADD(ntsc_sx)

	// devices
	MCFG_DEVICE_MODIFY("iec9")
	MCFG_DEVICE_SLOT_INTERFACE(sx1541_iec_devices, "sx1541", false)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc_c )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( ntsc_c, ntsc, c64c_state )
	MCFG_SOUND_REPLACE(MOS6581_TAG, MOS8580, XTAL_14_31818MHz/14)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c64_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c64_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal, c64_state )
	// basic hardware
	MCFG_CPU_ADD(M6510_TAG, M6510, XTAL_17_734472MHz/18)
	MCFG_CPU_PROGRAM_MAP(c64_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_M6510_PORT_CALLBACKS(READ8(c64_state, cpu_r), WRITE8(c64_state, cpu_w))
	MCFG_M6510_PORT_PULLS(0x17, 0xc8)
	MCFG_QUANTUM_PERFECT_CPU(M6510_TAG)

	// video hardware
	MCFG_DEVICE_ADD(MOS6569_TAG, MOS6569, XTAL_17_734472MHz/18)
	MCFG_MOS6566_CPU(M6510_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(c64_state, vic_irq_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6569_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6569_COLUMNS, VIC6569_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS6569_TAG, mos6569_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS6581, XTAL_17_734472MHz/18)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c64_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c64_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_DEVICE_ADD(MOS6526_1_TAG, MOS6526, XTAL_17_734472MHz/18)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c64_state, cia1_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_4))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_5))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c64_state, cia1_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c64_state, cia1_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c64_state, cia1_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c64_state, cia1_pb_w))
	MCFG_DEVICE_ADD(MOS6526_2_TAG, MOS6526, XTAL_17_734472MHz/18)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c64_state, cia2_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_6))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_7))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c64_state, cia2_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c64_state, cia2_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c64_state, cia2_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c64_state, cia2_pb_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_8))
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c1530", DEVWRITELINE(MOS6526_1_TAG, mos6526_device, flag_w))
	MCFG_CBM_IEC_ADD("c1541")
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, flag_w))
	MCFG_CBM_IEC_BUS_ATN_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_9))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS6569_TAG, mos6569_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, "joy")
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, XTAL_17_734472MHz/18, c64_expansion_cards, nullptr)
	MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(WRITELINE(c64_state, exp_irq_w))
	MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(WRITELINE(c64_state, exp_nmi_w))
	MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(WRITELINE(c64_state, exp_reset_w))
	MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(READ8(c64_state, read))
	MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(WRITE8(c64_state, write))
	MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(WRITELINE(c64_state, exp_dma_w))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, c64_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_3_HANDLER(WRITELINE(c64_state, exp_reset_w))
	MCFG_PET_USER_PORT_4_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_5_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_6_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_7_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_9_HANDLER(DEVWRITELINE(CBM_IEC_TAG, cbm_iec_device, atn_w))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_PET_USER_PORT_C_HANDLER(WRITELINE(c64_state, write_user_pb0))
	MCFG_PET_USER_PORT_D_HANDLER(WRITELINE(c64_state, write_user_pb1))
	MCFG_PET_USER_PORT_E_HANDLER(WRITELINE(c64_state, write_user_pb2))
	MCFG_PET_USER_PORT_F_HANDLER(WRITELINE(c64_state, write_user_pb3))
	MCFG_PET_USER_PORT_H_HANDLER(WRITELINE(c64_state, write_user_pb4))
	MCFG_PET_USER_PORT_J_HANDLER(WRITELINE(c64_state, write_user_pb5))
	MCFG_PET_USER_PORT_K_HANDLER(WRITELINE(c64_state, write_user_pb6))
	MCFG_PET_USER_PORT_L_HANDLER(WRITELINE(c64_state, write_user_pb7))
	MCFG_PET_USER_PORT_M_HANDLER(WRITELINE(c64_state, write_user_pa2))

	MCFG_QUICKLOAD_ADD("quickload", c64_state, cbm_c64, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "c64_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "c64_flop")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cass_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "PAL")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal_sx )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal_sx, sx64_state )
	MCFG_FRAGMENT_ADD(pal)

	// basic hardware
	MCFG_CPU_MODIFY(M6510_TAG)
	MCFG_M6510_PORT_CALLBACKS(READ8(sx64_state, cpu_r), WRITE8(sx64_state, cpu_w))
	MCFG_M6510_PORT_PULLS(0x07, 0xc0)

	// devices
	MCFG_DEVICE_MODIFY("iec8")
	MCFG_DEVICE_SLOT_INTERFACE(sx1541_iec_devices, "sx1541", false)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal_c )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( pal_c, pal, c64c_state )
	MCFG_SOUND_REPLACE(MOS6581_TAG, MOS8580, XTAL_17_734472MHz/18)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c64_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c64_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal_gs )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal_gs, c64gs_state )
	// basic hardware
	MCFG_CPU_ADD(M6510_TAG, M6510, XTAL_17_734472MHz/18)
	MCFG_CPU_PROGRAM_MAP(c64_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks
	MCFG_M6510_PORT_CALLBACKS(READ8(c64gs_state, cpu_r), WRITE8(c64gs_state, cpu_w))
	MCFG_M6510_PORT_PULLS(0x07, 0xc0)
	MCFG_QUANTUM_PERFECT_CPU(M6510_TAG)

	// video hardware
	MCFG_DEVICE_ADD(MOS6569_TAG, MOS8565, XTAL_17_734472MHz/18)
	MCFG_MOS6566_CPU(M6510_TAG)
	MCFG_MOS6566_IRQ_CALLBACK(WRITELINE(c64_state, vic_irq_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vic_videoram_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, vic_colorram_map)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6569_VRETRACERATE)
	MCFG_SCREEN_SIZE(VIC6569_COLUMNS, VIC6569_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MOS6569_TAG, mos8565_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6581_TAG, MOS8580, XTAL_17_734472MHz/18)
	MCFG_MOS6581_POTX_CALLBACK(READ8(c64_state, sid_potx_r))
	MCFG_MOS6581_POTY_CALLBACK(READ8(c64_state, sid_poty_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_DEVICE_ADD(MOS6526_1_TAG, MOS6526, XTAL_17_734472MHz/18)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c64_state, cia1_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_4))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_5))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c64gs_state, cia1_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c64_state, cia1_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c64gs_state, cia1_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c64_state, cia1_pb_w))
	MCFG_DEVICE_ADD(MOS6526_2_TAG, MOS6526, XTAL_17_734472MHz/18)
	MCFG_MOS6526_TOD(50)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c64_state, cia2_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_6))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_7))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c64_state, cia2_pa_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c64_state, cia2_pa_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c64_state, cia2_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c64_state, cia2_pb_w))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_8))
	MCFG_CBM_IEC_ADD(nullptr)
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, flag_w))
	MCFG_CBM_IEC_BUS_ATN_CALLBACK(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_9))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, nullptr)
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(DEVWRITELINE(MOS6569_TAG, mos6569_device, lp_w))
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, "joy")
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, XTAL_17_734472MHz/18, c64_expansion_cards, nullptr)
	MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(WRITELINE(c64_state, exp_irq_w))
	MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(WRITELINE(c64_state, exp_nmi_w))
	MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(WRITELINE(c64_state, exp_reset_w))
	MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(READ8(c64_state, read))
	MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(WRITE8(c64_state, write))
	MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(WRITELINE(c64_state, exp_dma_w))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, c64_user_port_cards, nullptr)
	MCFG_PET_USER_PORT_3_HANDLER(WRITELINE(c64_state, exp_reset_w))
	MCFG_PET_USER_PORT_4_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_5_HANDLER(DEVWRITELINE(MOS6526_1_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_6_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, cnt_w))
	MCFG_PET_USER_PORT_7_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, sp_w))
	MCFG_PET_USER_PORT_9_HANDLER(DEVWRITELINE(CBM_IEC_TAG, cbm_iec_device, atn_w))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(MOS6526_2_TAG, mos6526_device, flag_w))
	MCFG_PET_USER_PORT_C_HANDLER(WRITELINE(c64_state, write_user_pb0))
	MCFG_PET_USER_PORT_D_HANDLER(WRITELINE(c64_state, write_user_pb1))
	MCFG_PET_USER_PORT_E_HANDLER(WRITELINE(c64_state, write_user_pb2))
	MCFG_PET_USER_PORT_F_HANDLER(WRITELINE(c64_state, write_user_pb3))
	MCFG_PET_USER_PORT_H_HANDLER(WRITELINE(c64_state, write_user_pb4))
	MCFG_PET_USER_PORT_J_HANDLER(WRITELINE(c64_state, write_user_pb5))
	MCFG_PET_USER_PORT_K_HANDLER(WRITELINE(c64_state, write_user_pb6))
	MCFG_PET_USER_PORT_L_HANDLER(WRITELINE(c64_state, write_user_pb7))
	MCFG_PET_USER_PORT_M_HANDLER(WRITELINE(c64_state, write_user_pa2))

	MCFG_QUICKLOAD_ADD("quickload", c64_state, cbm_c64, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "PAL")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c64 )
//-------------------------------------------------

ROM_START( c64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r3")
	ROM_SYSTEM_BIOS(0, "r1", "Kernal rev. 1" )
	ROMX_LOAD( "901227-01.u4", 0x0000, 0x2000, CRC(dce782fa) SHA1(87cc04d61fc748b82df09856847bb5c2754a2033), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "r2", "Kernal rev. 2" )
	ROMX_LOAD( "901227-02.u4", 0x0000, 0x2000, CRC(a5c687b3) SHA1(0e2e4ee3f2d41f00bed72f9ab588b83e306fdb13), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "r3", "Kernal rev. 3" )
	ROMX_LOAD( "901227-03.u4", 0x0000, 0x2000, CRC(dbe3e7c7) SHA1(1d503e56df85a62fee696e7618dc5b4e781df1bb), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos c64.u4", 0x0000, 0x2000, CRC(2f79984c) SHA1(31e73e66eccb28732daea8ec3ad1addd9b39a017), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "speeddos", "SpeedDOS" )
	ROMX_LOAD( "speed-dos.u4", 0x0000, 0x2000, CRC(5beb9ac8) SHA1(8896c8de9e26ef1396eb46020b2de346a3eeab7e), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "speeddos20", "SpeedDOS-Plus+ v2.0" )
	ROMX_LOAD( "speed-dosplus.u4", 0x0000, 0x2000, CRC(10aee0ae) SHA1(6cebd4dc0c5e8c0b073586a3f1c43cc3349b9736), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "speeddos27", "SpeedDOS-Plus+ v2.7" )
	ROMX_LOAD( "speed-dosplus27.u4", 0x0000, 0x2000, CRC(ff59995e) SHA1(c8d864e5fc7089af8afce97dc0a0224df11df1c3), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "prodos", "Professional-DOS v1" )
	ROMX_LOAD( "prodos.u4", 0x0000, 0x2000, CRC(37ed83a2) SHA1(35f4f0fe03c0b7b3762b526ba855de41b496fb60), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(8, "prodos2", "Professional-DOS Release 2/4L2" )
	ROMX_LOAD( "prodos24l2.u4", 0x0000, 0x2000, CRC(41dad9fe) SHA1(fbf3dcc2ed40e58b07595740ea6fbff7ab19ebad), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(9, "prodos3", "Professional-DOS Release 3/5L2" )
	ROMX_LOAD( "prodos35l2.u4", 0x0000, 0x2000, CRC(2822eee7) SHA1(77356b84c1648018863d1c8dd5bc3a37485bc00e), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(10, "turborom", "Cockroach Turbo-ROM" )
	ROMX_LOAD( "turborom.u4", 0x0000, 0x2000, CRC(e6c763a2) SHA1(eff5a4b6bc65daa9421bd3856dd99a3195068e1c), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(11, "dosrom", "DOS-ROM v1.2" )
	ROMX_LOAD( "dosrom12.u4", 0x0000, 0x2000, CRC(ac030fc0) SHA1(0e4b38e81b49f55d52162154a44f0fffd2b0d04f), ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(12, "turborom2", "Datel Turbo ROM II (PAL)" )
	ROMX_LOAD( "turborom2.u4", 0x0000, 0x2000, CRC(ea3ba683) SHA1(4bb23f764a3d255119fbae37202ca820caa04e1f), ROM_BIOS(13) )
	ROM_SYSTEM_BIOS(13, "mercury", "Mercury-ROM v3 (NTSC)" )
	ROMX_LOAD( "mercury3.u4", 0x0000, 0x2000, CRC(6eac46a2) SHA1(4e351aa5fcb97c4c21e565aa2c830cc09bd47533), ROM_BIOS(14) )
	ROM_SYSTEM_BIOS(14, "dolphin", "Dolphin-DOS 1.0" )
	ROMX_LOAD( "kernal-10-mager.u4", 0x0000, 0x2000, CRC(c9bb21bc) SHA1(e305216e50ff8a7acf102be6c6343e3d44a16233), ROM_BIOS(15) )
	ROM_SYSTEM_BIOS(15, "dolphin201au", "Dolphin-DOS 2.0 1 au" )
	ROMX_LOAD( "kernal-20-1_au.u4", 0x0000, 0x2000, CRC(7068bbcc) SHA1(325ce7e32609a8fc704aaa76f5eb4cd7d8099a92), ROM_BIOS(16) )
	ROM_SYSTEM_BIOS(16, "dolphin201", "Dolphin-DOS 2.0 1" )
	ROMX_LOAD( "kernal-20-1.u4", 0x0000, 0x2000, CRC(c9c4c44e) SHA1(7f5d8f08c5ed2182ffb415a3d777fdd922496d02), ROM_BIOS(17) )
	ROM_SYSTEM_BIOS(17, "dolphin202", "Dolphin-DOS 2.0 2" )
	ROMX_LOAD( "kernal-20-2.u4", 0x0000, 0x2000, CRC(ffaeb9bc) SHA1(5f6c1bad379da16f77bccb58e80910f307dfd5f8), ROM_BIOS(18) )
	ROM_SYSTEM_BIOS(18, "dolphin203", "Dolphin-DOS 2.0 3" )
	ROMX_LOAD( "kernal-20-3.u4", 0x0000, 0x2000, CRC(4fd511f2) SHA1(316fba280dcb29496d593c0c4e3ee9a19844054e), ROM_BIOS(19) )
	ROM_SYSTEM_BIOS(19, "dolphin30", "Dolphin-DOS 3.0" )
	ROMX_LOAD( "kernal-30.u4", 0x0000, 0x2000, CRC(5402d643) SHA1(733acb96fead2fb4df77840c5bb618f08439fc7e), ROM_BIOS(20) )
	ROM_SYSTEM_BIOS(20, "taccess", "TurboAccess v2.6" )
	ROMX_LOAD( "turboaccess26.u4", 0x0000, 0x2000, CRC(93de6cd9) SHA1(a74478f3b9153c13176eac80ebfacc512ae7cbf0), ROM_BIOS(21) )
	ROM_SYSTEM_BIOS(21, "ttrans301", "TurboTrans v3.0 1" )
	ROMX_LOAD( "turboaccess301.u4", 0x0000, 0x2000, CRC(b3304dcf) SHA1(4d47a265ef65e4823f862cfc3d514c2a71473580), ROM_BIOS(22) )
	ROM_SYSTEM_BIOS(22, "ttrans302", "TurboTrans v3.0 2" )
	ROMX_LOAD( "turboaccess302.u4", 0x0000, 0x2000, CRC(9e696a7b) SHA1(5afae75d66d539f4bb4af763f029f0ef6523a4eb), ROM_BIOS(23) )
	ROM_SYSTEM_BIOS(23, "tprocess", "Turbo-Process (PAL)" )
	ROMX_LOAD( "turboprocess.u4", 0x0000, 0x2000, CRC(e5610d76) SHA1(e3f35777cfd16cce4717858f77ff354763395ba9), ROM_BIOS(24) )
	ROM_SYSTEM_BIOS(24, "tprocessn", "Turbo-Process (NTSC)" )
	ROMX_LOAD( "turboprocessus.u4", 0x0000, 0x2000, CRC(7480b76a) SHA1(ef1664b5057ae3cc6d104fc2f5c1fb29ee5a1b2b), ROM_BIOS(25) )
	ROM_SYSTEM_BIOS(25, "exos3", "EXOS v3" )
	ROMX_LOAD( "exos3.u4", 0x0000, 0x2000, CRC(4e54d020) SHA1(f8931b7c0b26807f4de0cc241f0b1e2c8f5271e9), ROM_BIOS(26) )
	ROM_SYSTEM_BIOS(26, "exos4", "EXOS v4" )
	ROMX_LOAD( "exos4.u4", 0x0000, 0x2000, CRC(d5cf83a9) SHA1(d5f03a5c0e9d00032d4751ecc6bcd6385879c9c7), ROM_BIOS(27) )
	ROM_SYSTEM_BIOS(27, "digidos", "DigiDOS" )
	ROMX_LOAD( "digidos.u4", 0x0000, 0x2000, CRC(2b0c8e89) SHA1(542d6f61c318bced0642e7c2d4d3b34a0f13e634), ROM_BIOS(28) )
	ROM_SYSTEM_BIOS(28, "magnum", "Magnum Load" )
	ROMX_LOAD( "magnum.u4", 0x0000, 0x2000, CRC(b2cffcc6) SHA1(827c782c1723b5d0992c05c00738ae4b2133b641), ROM_BIOS(29) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64_jp )
//-------------------------------------------------

ROM_START( c64_jp )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "906145-02.u4", 0x0000, 0x2000, CRC(3a9ef6f1) SHA1(4ff0f11e80f4b57430d8f0c3799ed0f0e0f4565d) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "906143-02.u5", 0x0000, 0x1000, CRC(1604f6c1) SHA1(0fad19dbcdb12461c99657b2979dbb5c2e47b527) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64p )
//-------------------------------------------------

#define rom_c64p rom_c64


//-------------------------------------------------
//  ROM( c64_se )
//-------------------------------------------------

ROM_START( c64_se )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernel.u4",  0x0000, 0x2000, CRC(f10c2c25) SHA1(e4f52d9b36c030eb94524eb49f6f0774c1d02e5e) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_SYSTEM_BIOS( 0, "default", "Swedish Characters" )
	ROMX_LOAD( "charswe.u5", 0x0000, 0x1000, CRC(bee9b3fd) SHA1(446ae58f7110d74d434301491209299f66798d8a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "alt", "Swedish Characters (Alt)" )
	ROMX_LOAD( "charswe2.u5", 0x0000, 0x1000, CRC(377a382b) SHA1(20df25e0ba1c88f31689c1521397c96968967fac), ROM_BIOS(2) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( pet64 )
//-------------------------------------------------

ROM_START( pet64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901246-01.u4", 0x0000, 0x2000, CRC(789c8cc5) SHA1(6c4fa9465f6091b174df27dfe679499df447503c) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( edu64 )
//-------------------------------------------------

#define rom_edu64   rom_c64


//-------------------------------------------------
//  ROM( sx64 )
//-------------------------------------------------

ROM_START( sx64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.ud4", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS(0, "cbm", "Original" )
	ROMX_LOAD( "251104-04.ud3", 0x0000, 0x2000, CRC(2c5965d4) SHA1(aa136e91ecf3c5ac64f696b3dbcbfc5ba0871c98), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos sx64.ud3", 0x0000, 0x2000, CRC(2b5a88f5) SHA1(942c2150123dc30f40b3df6086132ef0a3c43948), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "1541flash", "1541 FLASH!" )
	ROMX_LOAD( "1541 flash.ud3", 0x0000, 0x2000, CRC(0a1c9b85) SHA1(0bfcaab0ae453b663a6e01cd59a9764805419e00), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "turborom", "Cockroach Turbo-ROM" )
	ROMX_LOAD( "turboromsx.u4", 0x0000, 0x2000, CRC(48579c30) SHA1(6c907fdd07c14e162eb8c8fb750b1bbaf69dccb4), ROM_BIOS(4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.ud1", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( rom_sx64p )
//-------------------------------------------------

#define rom_sx64p   rom_sx64


//-------------------------------------------------
//  ROM( vip64 )
//-------------------------------------------------

ROM_START( vip64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.ud4", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernelsx.ud3", 0x0000, 0x2000, CRC(7858d3d7) SHA1(097cda60469492a8916c2677b7cce4e12a944bc0) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "charswe.ud1", 0x0000, 0x1000, CRC(bee9b3fd) SHA1(446ae58f7110d74d434301491209299f66798d8a) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( dx64 )
//-------------------------------------------------

// ROM_LOAD( "dx64kern.bin", 0x0000, 0x2000, CRC(58065128) ) TODO where is this illusive ROM?
#define rom_dx64    rom_sx64


//-------------------------------------------------
//  ROM( tesa6240 )
//-------------------------------------------------

ROM_START( tesa6240 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "tesa-basic.ud4", 0x0000, 0x2000, CRC(f319d661) SHA1(0033afa7d2fbff314d80427324633c5444fbf1cd) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "tesa-kernal.ud3", 0x0000, 0x2000, CRC(af638f9c) SHA1(a2c9c83f598623c9940949979ac643f12397e907) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "tesa-char.ud1", 0x0000, 0x1000, CRC(10765a90) SHA1(1b824df5a295d0479e830e272758640b9fe99344) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64c )
//-------------------------------------------------

ROM_START( c64c )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_DEFAULT_BIOS("cbm")
	ROM_SYSTEM_BIOS(0, "cbm", "Original" )
	ROMX_LOAD( "251913-01.u4", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "pdc", "ProLogic-DOS Classic" )
	ROMX_LOAD( "pdc.u4", 0x0000, 0x4000, CRC(6b653b9c) SHA1(0f44a9c62619424a0cd48a90e1b377b987b494e0), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64cp )
//-------------------------------------------------

#define rom_c64cp       rom_c64c


//-------------------------------------------------
//  ROM( c64g )
//-------------------------------------------------

#define rom_c64g        rom_c64c


//-------------------------------------------------
//  ROM( c64c_es )
//-------------------------------------------------

ROM_START( c64c_es )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_LOAD( "251913-01.u4", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "325056-03.u5", 0x0000, 0x1000, CRC(c890c175) SHA1(4f57259fff9ef1963a4e87165a6f35ca23864c76) ) // aka 325245-01

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64c_se )
//-------------------------------------------------

ROM_START( c64c_se )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_LOAD( "325182-01.u4", 0x0000, 0x4000, CRC(2aff27d3) SHA1(267654823c4fdf2167050f41faa118218d2569ce) ) // 128/64 FI

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "cbm 64 skand.gen.u5", 0x0000, 0x1000, CRC(377a382b) SHA1(20df25e0ba1c88f31689c1521397c96968967fac) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64gs )
//-------------------------------------------------

ROM_START( c64gs )
	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_LOAD( "390852-01.u4", 0x0000, 0x4000, CRC(b0a9c2da) SHA1(21940ef5f1bfe67d7537164f7ca130a1095b067a) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252535-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   INIT                        COMPANY                        FULLNAME                                     FLAGS
COMP( 1982, c64,     0,      0,      ntsc,       c64,    driver_device,      0,      "Commodore Business Machines", "Commodore 64 (NTSC)",                       MACHINE_SUPPORTS_SAVE )
COMP( 1982, c64_jp,  c64,    0,      ntsc,       c64,    driver_device,      0,      "Commodore Business Machines", "Commodore 64 (Japan)",                      MACHINE_SUPPORTS_SAVE )
COMP( 1982, c64p,    c64,    0,      pal,        c64,    driver_device,      0,      "Commodore Business Machines", "Commodore 64 (PAL)",                        MACHINE_SUPPORTS_SAVE )
COMP( 1982, c64_se,  c64,    0,      pal,        c64sw,  driver_device,      0,      "Commodore Business Machines", "Commodore 64 / VIC-64S (Sweden/Finland)",   MACHINE_SUPPORTS_SAVE )
COMP( 1983, pet64,   c64,    0,      pet64,      c64,    driver_device,      0,      "Commodore Business Machines", "PET 64 / CBM 4064 (NTSC)",                  MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS )
COMP( 1983, edu64,   c64,    0,      pet64,      c64,    driver_device,      0,      "Commodore Business Machines", "Educator 64 (NTSC)",                        MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS )
COMP( 1984, sx64,    c64,    0,      ntsc_sx,    c64,    driver_device,      0,      "Commodore Business Machines", "SX-64 / Executive 64 (NTSC)",               MACHINE_SUPPORTS_SAVE )
COMP( 1984, sx64p,   c64,    0,      pal_sx,     c64,    driver_device,      0,      "Commodore Business Machines", "SX-64 / Executive 64 (PAL)",                MACHINE_SUPPORTS_SAVE )
COMP( 1984, vip64,   c64,    0,      pal_sx,     c64sw,  driver_device,      0,      "Commodore Business Machines", "VIP-64 (Sweden/Finland)",                   MACHINE_SUPPORTS_SAVE )
COMP( 1984, dx64,    c64,    0,      ntsc_dx,    c64,    driver_device,      0,      "Commodore Business Machines", "DX-64 (NTSC)",                              MACHINE_SUPPORTS_SAVE )
COMP( 1984, tesa6240,c64,    0,      pal_sx,     c64,    driver_device,      0,      "Tesa Etikett",                "Etikettendrucker 6240",                     MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64c,    c64,    0,      ntsc_c,     c64,    driver_device,      0,      "Commodore Business Machines", "Commodore 64C (NTSC)",                      MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64cp,   c64,    0,      pal_c,      c64,    driver_device,      0,      "Commodore Business Machines", "Commodore 64C (PAL)",                       MACHINE_SUPPORTS_SAVE )
COMP( 1988, c64c_es, c64,    0,      pal_c,      c64sw,  driver_device,      0,      "Commodore Business Machines", "Commodore 64C (Spain)",                     MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64c_se, c64,    0,      pal_c,      c64sw,  driver_device,      0,      "Commodore Business Machines", "Commodore 64C (Sweden/Finland)",            MACHINE_SUPPORTS_SAVE )
COMP( 1986, c64g,    c64,    0,      pal_c,      c64,    driver_device,      0,      "Commodore Business Machines", "Commodore 64G (PAL)",                       MACHINE_SUPPORTS_SAVE )
CONS( 1990, c64gs,   c64,    0,      pal_gs,     c64gs,  driver_device,      0,      "Commodore Business Machines", "Commodore 64 Games System (PAL)",           MACHINE_SUPPORTS_SAVE )
