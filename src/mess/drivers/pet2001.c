/*

	TODO:

	- cursor
	- accurate video timings
	- user port
	- memory expansion port

*/

#include "includes/pet2001.h"



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void pet2001_state::check_interrupts()
{
	int irq = m_via_irq || m_pia1a_irq || m_pia1b_irq || m_pia2a_irq || m_pia2b_irq || m_exp_irq;

	m_maincpu->set_input_line(M6502_IRQ_LINE, irq);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( pet2001_state::read )
{
	UINT8 data = 0;

	switch (offset >> 12)
	{
	case SEL0:
	case SEL1:
	case SEL2:
	case SEL3:
	case SEL4:
	case SEL5:
	case SEL6:
	case SEL7:
		if (offset < m_ram->size())
		{
			data = m_ram->pointer()[offset];
		}
		break;

	case SEL8:
		data = m_video_ram[offset & 0x3ff];
		break;

	case SEL9:
		if (m_spare_rom)
		{
			data = m_spare_rom->base()[offset & 0xfff];
		}
		break;

	case SELA:
		if (m_spare_rom)
		{
			data = m_spare_rom->base()[0x1000 | (offset & 0xfff)];
		}
		break;

	case SELB:
		if (m_spare_rom)
		{
			data = m_spare_rom->base()[0x2000 | (offset & 0xfff)];
		}
		break;
	
	case SELE:
		if (BIT(offset, 11))
		{
			if (BIT(offset, 4))
			{
				data = m_pia1->read(space, offset & 0x03);
			}
			if (BIT(offset, 5))
			{
				data = m_pia2->read(space, offset & 0x03);
			}
			if (BIT(offset, 6))
			{
				data = m_via->read(space, offset & 0x0f);
			}
		}
		else
		{
			data = m_rom->base()[offset & 0x3fff];
		}
		break;

	case SELC:
	case SELD:
	case SELF:
		data = m_rom->base()[offset & 0x3fff];
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( pet2001_state::write )
{
	switch (offset >> 12)
	{
	case SEL0:
	case SEL1:
	case SEL2:
	case SEL3:
	case SEL4:
	case SEL5:
	case SEL6:
	case SEL7:
		if (offset < m_ram->size())
		{
			m_ram->pointer()[offset] = data;
		}
		break;

	case SEL8:
		m_video_ram[offset & 0x3ff] = data;
		break;

	case SELE:
		if (BIT(offset, 11))
		{
			if (BIT(offset, 4))
			{
				m_pia1->write(space, offset & 0x03, data);
			}
			if (BIT(offset, 5))
			{
				m_pia2->write(space, offset & 0x03, data);
			}
			if (BIT(offset, 6))
			{
				m_via->write(space, offset & 0x0f, data);
			}
		}
		break;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( pet2001_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( pet2001_mem, AS_PROGRAM, 8, pet2001_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( pet )
//-------------------------------------------------

static INPUT_PORTS_START( pet )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home  Clr Screen") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_MINUS) PORT_CHAR(0x2190)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('!')

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del  Inst") PORT_CODE(KEYCODE_DEL) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('"')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR('9')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR('7')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91 Pi") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR('/')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('W')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('A')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR('5')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('S')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR(';')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('X')

	PORT_START( "ROW8" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR('-')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR('0')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START( "ROW9" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop Run") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('[')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Rvs Off") PORT_CODE(KEYCODE_TAB)

	PORT_START( "LOCK" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( petb )
//-------------------------------------------------

INPUT_PORTS_START( petb )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('\\')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x2191)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('S')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('A')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del  Inst") PORT_CODE(KEYCODE_DEL) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Repeat") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')

	PORT_START( "ROW8" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home  Clr Screen") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Rvs Off") PORT_CODE(KEYCODE_INSERT)

	PORT_START( "ROW9" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop Run") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x2190)

	PORT_START( "LOCK" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  via6522_interface via_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( pet2001_state::via_irq_w )
{
	m_via_irq = state;

	check_interrupts();
}

READ8_MEMBER( pet2001_state::via_pb_r )
{
	/*

	    bit     description

	    PB0     _NDAC IN
	    PB1     
	    PB2     
	    PB3     
	    PB4     
	    PB5     SYNC IN
	    PB6     _NRFD IN
	    PB7     _DAV IN

	*/

	UINT8 data = 0;

	// video sync
	data |= m_sync << 5;

	// IEEE-488
	data |= m_ieee->ndac_r();
	data |= m_ieee->nrfd_r() << 6;
	data |= m_ieee->dav_r() << 7;

	return data;
}

WRITE8_MEMBER( pet2001_state::via_pb_w )
{
	/*

	    bit     description

	    PB0     
	    PB1     _NRFD OUT
	    PB2     _ATN OUT
	    PB3     CASS WRITE
	    PB4     #2 CASS MOTOR
	    PB5     
	    PB6     
	    PB7     

	*/

	// IEEE-488
	m_ieee->nrfd_w(BIT(data, 1));
	m_ieee->atn_w(BIT(data, 2));

	// cassette
	m_cassette->write(BIT(data, 3));
	m_cassette2->write(BIT(data, 3));
	m_cassette2->motor_w(BIT(data, 4));
}

WRITE_LINE_MEMBER( pet2001_state::via_ca2_w )
{
	m_graphic = state;
}

const via6522_interface via_intf =
{
	DEVCB_NULL,//DEVCB_DEVICE_MEMBER(PET_USER_PORT_TAG, pet_user_port_device, pa_r),
	DEVCB_DRIVER_MEMBER(pet2001_state, via_pb_r),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(PET_DATASSETTE_PORT2_TAG, pet_datassette_port_device, read),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,//DEVCB_DEVICE_MEMBER(PET_USER_PORT_TAG, pet_user_port_device, pa_w),
	DEVCB_DRIVER_MEMBER(pet2001_state, via_pb_w),
	DEVCB_NULL,//DEVCB_DEVICE_LINE_MEMBER(PET_USER_PORT_TAG, pet_user_port_device, ca1_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, via_ca2_w),
	DEVCB_NULL,
	DEVCB_NULL,//DEVCB_DEVICE_LINE_MEMBER(PET_USER_PORT_TAG, pet_user_port_device, cb2_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, via_irq_w)
};


//-------------------------------------------------
//  pia6821_interface pia1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( pet2001_state::pia1_irqa_w )
{
	m_pia1a_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( pet2001_state::pia1_irqb_w )
{
	m_pia1b_irq = state;

	check_interrupts();
}

READ8_MEMBER( pet2001_state::pia1_pa_r )
{
	/*

	    bit     description

	    PA0     KEY A
	    PA1     KEY B
	    PA2     KEY C
	    PA3     KEY D
	    PA4     #1 CASS SWITCH
	    PA5     #2 CASS SWITCH
	    PA6     _EOI IN
	    PA7     DIAG JUMPER

	*/

	UINT8 data = 0;

	// keyboard
	data |= m_key;

	// cassette
	data |= m_cassette->sense_r() << 4;
	data |= m_cassette2->sense_r() << 5;

	// IEEE-488
	data |= m_ieee->eoi_r() << 6;

	// diagnostic jumper
	data |= 0x80;

	return data;
}

WRITE8_MEMBER( pet2001_state::pia1_pa_w )
{
	/*

	    bit     description

	    PA0     KEY A
	    PA1     KEY B
	    PA2     KEY C
	    PA3     KEY D
	    PA4     
	    PA5     
	    PA6     
	    PA7     

	*/

	// keyboard
	m_key = data & 0x0f;
}

READ8_MEMBER( pet2001_state::pia1_pb_r )
{
	UINT8 data = 0xff;

	switch (m_key)
	{
	case 0: data &= m_row0->read(); break;
	case 1: data &= m_row1->read(); break;
	case 2: data &= m_row2->read(); break;
	case 3: data &= m_row3->read(); break;
	case 4: data &= m_row4->read(); break;
	case 5: data &= m_row5->read(); break;
	case 6: data &= m_row6->read(); break;
	case 7: data &= m_row7->read(); break;
	case 8: data &= m_row8->read() & m_lock->read(); break;
	case 9: data &= m_row9->read(); break;
	}

	return data;
}

READ8_MEMBER( pet2001b_state::pia1_pb_r )
{
	UINT8 data = 0xff;

	switch (m_key)
	{
	case 0: data &= m_row0->read(); break;
	case 1: data &= m_row1->read(); break;
	case 2: data &= m_row2->read(); break;
	case 3: data &= m_row3->read(); break;
	case 4: data &= m_row4->read(); break;
	case 5: data &= m_row5->read(); break;
	case 6: data &= m_row6->read() & m_lock->read(); break;
	case 7: data &= m_row7->read(); break;
	case 8: data &= m_row8->read(); break;
	case 9: data &= m_row9->read(); break;
	}

	return data;
}

READ_LINE_MEMBER( pet2001_state::pia1_cb1_r )
{
	return m_sync;
}

WRITE_LINE_MEMBER( pet2001_state::pia1_ca2_w )
{
	m_ieee->eoi_w(state);

	m_blanktv = state;
}

const pia6821_interface pia1_intf =
{
	DEVCB_DRIVER_MEMBER(pet2001_state, pia1_pa_r),
	DEVCB_DRIVER_MEMBER(pet2001_state, pia1_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, read),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_cb1_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(pet2001_state, pia1_pa_w),
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_ca2_w),
	DEVCB_DEVICE_LINE_MEMBER(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, motor_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_irqa_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_irqb_w)
};

const pia6821_interface pet2001b_pia1_intf =
{
	DEVCB_DRIVER_MEMBER(pet2001_state, pia1_pa_r),
	DEVCB_DRIVER_MEMBER(pet2001b_state, pia1_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, read),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_cb1_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(pet2001_state, pia1_pa_w),
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_ca2_w),
	DEVCB_DEVICE_LINE_MEMBER(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, motor_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_irqa_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia1_irqb_w)
};


//-------------------------------------------------
//  pia6821_interface pia2_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( pet2001_state::pia2_irqa_w )
{
	m_pia2a_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( pet2001_state::pia2_irqb_w )
{
	m_pia2b_irq = state;

	check_interrupts();
}

const pia6821_interface pia2_intf =
{
	DEVCB_DEVICE_MEMBER(IEEE488_TAG, ieee488_device, dio_r),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(IEEE488_TAG, ieee488_device, atn_r),
	DEVCB_DEVICE_LINE_MEMBER(IEEE488_TAG, ieee488_device, srq_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(IEEE488_TAG, ieee488_device, dio_w),
	DEVCB_DEVICE_LINE_MEMBER(IEEE488_TAG, ieee488_device, ndac_w),
	DEVCB_DEVICE_LINE_MEMBER(IEEE488_TAG, ieee488_device, dav_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia2_irqa_w),
	DEVCB_DRIVER_LINE_MEMBER(pet2001_state, pia2_irqb_w)
};


//-------------------------------------------------
//  IEEE488_INTERFACE( ieee488_intf )
//-------------------------------------------------

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(M6520_2_TAG, pia6821_device, cb1_w),
	DEVCB_DEVICE_LINE_MEMBER(M6520_2_TAG, pia6821_device, ca1_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
//-------------------------------------------------

static PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(M6520_1_TAG, pia6821_device, ca1_w)
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette2_intf )
//-------------------------------------------------

static PET_DATASSETTE_PORT_INTERFACE( datassette2_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(M6522_TAG, via6522_device, write_cb1)
};



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( sync_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( pet2001_state::sync_tick )
{
	m_sync = !m_sync;

	m_pia1->cb1_w(m_sync);
}


//-------------------------------------------------
//  SCREEN_UPDATE( pet2001 )
//-------------------------------------------------

UINT32 pet2001_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 200; y++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			int sy = y / 8;
			offs_t video_addr = (sy * 40) + sx;
			UINT8 code = m_video_ram[video_addr];

			int ra = y & 0x07;
			offs_t char_addr = (m_graphic << 10) | (code << 3) | ra;
			UINT8 data = m_char_rom->base()[char_addr];

			for (int x = 0; x < 8; x++)
			{
				int color = BIT(data, 7);

				bitmap.pix32(y, (sx * 8) + x) = RGB_MONOCHROME_GREEN[color];
				
				data <<= 1;
			}
		}
	}

	return 0;
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( pet2001 )
//-------------------------------------------------

void pet2001_state::machine_start()
{
	// allocate memory
	m_video_ram.allocate(0x400);

	// initialize memory
	UINT8 data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	data = 0xff;

	for (offs_t offset = 0; offset < 0x400; offset++)
	{
		m_video_ram[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_key));
	save_item(NAME(m_sync));
	save_item(NAME(m_graphic));
	save_item(NAME(m_blanktv));
	save_item(NAME(m_via_irq));
	save_item(NAME(m_pia1a_irq));
	save_item(NAME(m_pia1b_irq));
	save_item(NAME(m_pia2a_irq));
	save_item(NAME(m_pia2b_irq));
	save_item(NAME(m_exp_irq));
}


//-------------------------------------------------
//  MACHINE_RESET( pet2001 )
//-------------------------------------------------

void pet2001_state::machine_reset()
{
	m_maincpu->reset();

	m_via->reset();
	m_pia1->reset();
	m_pia2->reset();
	//m_exp->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( pet2001 )
//-------------------------------------------------

static MACHINE_CONFIG_START( pet2001, pet2001_state )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_8MHz/8)
	MCFG_CPU_PROGRAM_MAP(pet2001_mem)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(pet2001_state, screen_update)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sync_timer", pet2001_state, sync_tick, attotime::from_hz(120))

	// devices
	MCFG_VIA6522_ADD(M6522_TAG, XTAL_8MHz/8, via_intf)
	MCFG_PIA6821_ADD(M6520_1_TAG, pia1_intf)
	MCFG_PIA6821_ADD(M6520_2_TAG, pia2_intf)
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c4040")
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT2_TAG, datassette2_intf, cbm_datassette_devices, NULL, NULL)
	//MCFG_QUICKLOAD_ADD("quickload", cbm_pet, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	//MCFG_PET_EXPANSION_SLOT_ADD(PET_EXPANSION_SLOT_TAG, XTAL_8MHz/8, pet_expansion_cards, NULL, NULL)
	//MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, user_intf, pet_user_port_cards, NULL, NULL)

	// internal RAM
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("8K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pet_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001n )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet2001n, pet2001 )
	MCFG_CARTSLOT_ADD("9000")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_INTERFACE("pet_9000_rom")

	MCFG_CARTSLOT_ADD("a000")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_INTERFACE("pet_a000_rom")

	MCFG_CARTSLOT_ADD("b000")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_INTERFACE("pet_b000_rom")

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("16K,32K")

	MCFG_SOFTWARE_LIST_ADD("rom_list", "pet_rom")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet2001b )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( pet2001b, pet2001n, pet2001b_state )
	MCFG_DEVICE_REMOVE(M6520_1_TAG)
	MCFG_PIA6821_ADD(M6520_1_TAG, pet2001b_pia1_intf)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pet2001 )
//-------------------------------------------------

ROM_START( pet2001 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS( "basic1r" )
	ROM_SYSTEM_BIOS( 0, "basic1o", "Original" )
	ROMX_LOAD( "901447-01.h1", 0x0000, 0x0800, CRC(a055e33a) SHA1(831db40324113ee996c434d38b4add3fd1f820bd), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic1r", "Revised" )
	ROMX_LOAD( "901447-09.h1", 0x0000, 0x0800, CRC(03cf16d0) SHA1(1330580c0614d3556a389da4649488ba04a60908), ROM_BIOS(2) )
	ROM_LOAD( "901447-02.h5", 0x0800, 0x0800, CRC(69fd8a8f) SHA1(70c0f4fa67a70995b168668c957c3fcf2c8641bd) )
	ROM_LOAD( "901447-03.h2", 0x1000, 0x0800, CRC(d349f2d4) SHA1(4bf2c20c51a63d213886957485ebef336bb803d0) )
	ROM_LOAD( "901447-04.h6", 0x1800, 0x0800, CRC(850544eb) SHA1(d293972d529023d8fd1f493149e4777b5c253a69) )
	ROM_LOAD( "901447-05.h3", 0x2000, 0x0800, CRC(9e1c5cea) SHA1(f02f5fb492ba93dbbd390f24c10f7a832dec432a) )
	ROM_LOAD( "901447-06.h4", 0x3000, 0x0800, CRC(661a814a) SHA1(960717282878e7de893d87242ddf9d1512be162e) )
	ROM_LOAD( "901447-07.h7", 0x3800, 0x0800, CRC(c4f47ad1) SHA1(d440f2510bc52e20c3d6bc8b9ded9cea7f462a9c) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "901447-08.a2", 0x000, 0x800, CRC(54f32f45) SHA1(3e067cc621e4beafca2b90cb8f6dba975df2855b) )
ROM_END


//-------------------------------------------------
//  ROM( pet2001n )
//-------------------------------------------------

ROM_START( pet2001n )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "901465-01.ud6", 0x0000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )   // BASIC 2
	ROM_LOAD( "901465-02.ud7", 0x1000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )   // BASIC 2
	ROM_LOAD( "901447-24.ud8", 0x2000, 0x0800, CRC(e459ab32) SHA1(5e5502ce32f5a7e387d65efe058916282041e54b) )   // Screen Editor (40 columns, no CRTC, Normal Keyb)
	ROM_LOAD( "901465-03.ud9", 0x3000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )   // Kernal

	ROM_REGION( 0x3000, "spare", ROMREGION_ERASE00 )
	ROM_CART_LOAD( "9000", 0x0000, 0x1000, ROM_MIRROR )
	ROM_CART_LOAD( "a000", 0x1000, 0x1000, ROM_MIRROR )
	ROM_CART_LOAD( "b000", 0x2000, 0x1000, ROM_MIRROR )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END


//-------------------------------------------------
//  ROM( pet2001b )
//-------------------------------------------------

ROM_START( pet2001b )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "901465-01.ud6", 0x0000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )   // BASIC 2
	ROM_LOAD( "901465-02.ud7", 0x1000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )   // BASIC 2
	ROM_LOAD( "901474-01.ud8", 0x2000, 0x0800, CRC(05db957e) SHA1(174ace3a8c0348cd21d39cc864e2adc58b0101a9) )   // Screen Editor (40 columns, no CRTC, Business Keyb)
	ROM_LOAD( "901465-03.ud9", 0x3000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )   // Kernal

	ROM_REGION( 0x3000, "spare", ROMREGION_ERASE00 )
	ROM_CART_LOAD( "9000", 0x0000, 0x1000, ROM_MIRROR )
	ROM_CART_LOAD( "a000", 0x1000, 0x1000, ROM_MIRROR )
	ROM_CART_LOAD( "b000", 0x2000, 0x1000, ROM_MIRROR )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "901447-10.uf10", 0x000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )   // Character Generator
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   INIT                        COMPANY                        FULLNAME                                     FLAGS
COMP( 1977, pet2001,  0,        0,        pet2001,       pet, driver_device,      0,     "Commodore Business Machines",  "PET 2001", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
COMP( 1979, pet2001n, pet2001,  0,        pet2001n,      pet, driver_device,      0,     "Commodore Business Machines",  "PET 2001-N", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
COMP( 1979, pet2001b, pet2001,  0,        pet2001b,     petb, driver_device,      0,     "Commodore Business Machines",  "PET 2001-B", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
