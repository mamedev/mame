// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*
    TODO:

    - C1540 is not working currently
    - mos6560_port_r/w should respond at 0x1000-0x100f
    - VIC21 (built in 21K ram)

*/

#include "includes/vic20.h"



QUICKLOAD_LOAD_MEMBER( vic20_state, cbm_vc20 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}

//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( vic20_state::read )
{
	UINT8 data = m_vic->bus_r();

	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	switch ((offset >> 13) & 0x07)
	{
	case BLK0:
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			data = m_ram->pointer()[offset & 0x3ff];
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			data = m_ram->pointer()[0x400 + (offset & 0xfff)];
			break;
		}
		break;

	case BLK1: blk1 = 0; break;
	case BLK2: blk2 = 0; break;
	case BLK3: blk3 = 0; break;

	case BLK4:
		switch ((offset >> 10) & 0x07)
		{
		default:
			data = m_charom[offset & 0xfff];
			break;

		case IO0:
			if (BIT(offset, 4))
			{
				data = m_via1->read(space, offset & 0x0f);
			}
			else if (BIT(offset, 5))
			{
				data = m_via2->read(space, offset & 0x0f);
			}
			else if (offset >= 0x9000 && offset < 0x9010)
			{
				data = m_vic->read(space, offset & 0x0f);
			}
			break;

		case COLOR:
			data = m_color_ram[offset & 0x3ff];
			break;

		case IO2: io2 = 0; break;
		case IO3: io3 = 0; break;
		}
		break;

	case BLK5: blk5 = 0; break;

	case BLK6:
		data = m_basic[offset & 0x1fff];
		break;

	case BLK7:
		data = m_kernal[offset & 0x1fff];
		break;
	}

	return m_exp->cd_r(space, offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( vic20_state::write )
{
	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	switch ((offset >> 13) & 0x07)
	{
	case BLK0:
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			m_ram->pointer()[offset] = data;
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			m_ram->pointer()[0x400 + (offset & 0xfff)] = data;
			break;
		}
		break;

	case BLK1: blk1 = 0; break;
	case BLK2: blk2 = 0; break;
	case BLK3: blk3 = 0; break;

	case BLK4:
		switch ((offset >> 10) & 0x07)
		{
		case IO0:
			if (BIT(offset, 4))
			{
				m_via1->write(space, offset & 0x0f, data);
			}
			else if (BIT(offset, 5))
			{
				m_via2->write(space, offset & 0x0f, data);
			}
			else if (offset >= 0x9000 && offset < 0x9010)
			{
				m_vic->write(space, offset & 0x0f, data);
			}
			break;

		case COLOR:
			m_color_ram[offset & 0x3ff] = data & 0x0f;
			break;

		case IO2: io2 = 0; break;
		case IO3: io3 = 0; break;
		}
		break;

	case BLK5: blk5 = 0; break;
	}

	m_exp->cd_w(space, offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( vic20_state::vic_videoram_r )
{
	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	UINT8 data = 0;

	if (BIT(offset, 13))
	{
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			data = m_ram->pointer()[offset & 0x3ff];
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			data = m_ram->pointer()[0x400 + (offset & 0xfff)];
			break;
		}
	}
	else
	{
		data = m_charom[offset & 0xfff];
	}

	return m_exp->cd_r(space, offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vic20_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( vic20_mem, AS_PROGRAM, 8, vic20_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, vic20_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, vic20_state )
	AM_RANGE(0x000, 0x3ff) AM_RAM AM_SHARE("color_ram")
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vic20 )
//-------------------------------------------------

static INPUT_PORTS_START( vic20 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del  Inst") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)     PORT_CHAR('\xA3')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)          PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)              PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)              PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)              PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)              PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)              PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)     PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)              PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)              PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)              PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)              PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)              PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x2190)

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)          PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)              PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)              PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)              PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)              PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)              PORT_CHAR('A')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)            PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)          PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)          PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)              PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)              PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)              PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop Run") PORT_CODE(KEYCODE_HOME)

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)             PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)           PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)              PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)              PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)              PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)              PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)          PORT_CHAR(' ')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)             PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)      PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)          PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)              PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)              PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)              PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)              PORT_CHAR('S')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)             PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)      PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)              PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)              PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)              PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)              PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)              PORT_CHAR('Q')

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)             PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home  Clr") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)         PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)              PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)              PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)              PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)              PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)              PORT_CHAR('2') PORT_CHAR('"')

	PORT_START( "RESTORE" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_PRTSCR) PORT_WRITE_LINE_DEVICE_MEMBER(M6522_1_TAG, via6522_device, write_ca1)

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic1001 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1001 )
	PORT_INCLUDE( vic20 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('\xA5')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic20s )
//-------------------------------------------------

static INPUT_PORTS_START( vic20s )
	PORT_INCLUDE( vic20 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)          PORT_CHAR('-')

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)     PORT_CHAR('@')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)          PORT_CHAR(0x00C4)

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)          PORT_CHAR(0x00D6)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)      PORT_CHAR(0x00C5)

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)         PORT_CHAR('=')
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

READ8_MEMBER( vic20_state::via1_pa_r )
{
	/*

	    bit     description

	    PA0     SERIAL CLK IN
	    PA1     SERIAL DATA IN
	    PA2     JOY 0 (UP)
	    PA3     JOY 1 (DOWN)
	    PA4     JOY 2 (LEFT)
	    PA5     LITE PEN (FIRE)
	    PA6     CASS SWITCH
	    PA7

	*/

	UINT8 data = 0;

	// serial clock in
	data |= m_iec->clk_r();

	// serial data in
	data |= m_iec->data_r() << 1;

	// joystick / user port
	UINT8 joy = m_joy->joy_r();

	data |= (m_user_joy0 && BIT(joy, 0)) << 2;
	data |= (m_user_joy1 && BIT(joy, 1)) << 3;
	data |= (m_user_joy2 && BIT(joy, 2)) << 4;
	data |= (m_user_light_pen && BIT(joy, 5)) << 5;

	// cassette switch
	data |= (m_user_cassette_switch && m_cassette->sense_r()) << 6;

	return data;
}

WRITE8_MEMBER( vic20_state::via1_pa_w )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2
	    PA3
	    PA4
	    PA5     LITE PEN (FIRE)
	    PA6
	    PA7     SERIAL ATN OUT

	*/

	// light pen strobe
	m_user->write_7(BIT(data, 5));
	m_vic->lp_w(BIT(data, 5));

	// serial attention out
	m_user->write_9(!BIT(data, 7));
	m_iec->atn_w(!BIT(data, 7));
}

WRITE8_MEMBER( vic20_state::via1_pb_w )
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

READ8_MEMBER( vic20_state::via2_pa_r )
{
	/*

	    bit     description

	    PA0     ROW 0
	    PA1     ROW 1
	    PA2     ROW 2
	    PA3     ROW 3
	    PA4     ROW 4
	    PA5     ROW 5
	    PA6     ROW 6
	    PA7     ROW 7

	*/

	UINT8 data = 0xff;

	if (!BIT(m_key_col, 0)) data &= m_row0->read();
	if (!BIT(m_key_col, 1)) data &= m_row1->read();
	if (!BIT(m_key_col, 2)) data &= m_row2->read();
	if (!BIT(m_key_col, 3)) data &= m_row3->read();
	if (!BIT(m_key_col, 4)) data &= m_row4->read();
	if (!BIT(m_key_col, 5)) data &= m_row5->read();
	if (!BIT(m_key_col, 6)) data &= m_row6->read();
	if (!BIT(m_key_col, 7)) data &= m_row7->read();

	return data;
}

READ8_MEMBER( vic20_state::via2_pb_r )
{
	/*

	    bit     description

	    PB0     COL 0
	    PB1     COL 1
	    PB2     COL 2
	    PB3     COL 3
	    PB4     COL 4
	    PB5     COL 5
	    PB6     COL 6
	    PB7     COL 7, JOY 3 (RIGHT)

	*/

	UINT8 data = 0xff;

	// joystick
	UINT8 joy = m_joy->joy_r();

	data &= BIT(joy, 3) << 7;

	return data;
}

WRITE8_MEMBER( vic20_state::via2_pb_w )
{
	/*

	    bit     description

	    PB0     COL 0
	    PB1     COL 1
	    PB2     COL 2
	    PB3     COL 3, CASS WRITE
	    PB4     COL 4
	    PB5     COL 5
	    PB6     COL 6
	    PB7     COL 7

	*/

	// cassette write
	m_cassette->write(BIT(data, 3));

	// keyboard column
	m_key_col = data;
}

WRITE_LINE_MEMBER( vic20_state::via2_ca2_w )
{
	// serial clock out
	m_iec->clk_w(!state);
}

WRITE_LINE_MEMBER( vic20_state::via2_cb2_w )
{
	// serial data out
	m_iec->data_w(!state);
}


//-------------------------------------------------
//  VIC20_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vic20_state::exp_reset_w )
{
	if (!state)
	{
		machine_reset();
	}
}


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( vic20 )
//-------------------------------------------------

void vic20_state::machine_start()
{
	// initialize memory
	UINT8 data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_key_col));
	save_item(NAME(m_light_pen));
	save_item(NAME(m_user_joy0));
	save_item(NAME(m_user_joy1));
	save_item(NAME(m_user_joy2));
	save_item(NAME(m_user_light_pen));
	save_item(NAME(m_user_cassette_switch));
}


void vic20_state::machine_reset()
{
	m_maincpu->reset();

	m_vic->reset();
	m_via1->reset();
	m_via2->reset();

	m_iec->reset();
	m_exp->reset();

	m_user->write_3(0);
	m_user->write_3(1);
}

WRITE_LINE_MEMBER(vic20_state::write_user_joy0)
{
	m_user_joy0 = state;
}

WRITE_LINE_MEMBER(vic20_state::write_user_joy1)
{
	m_user_joy1 = state;
}

WRITE_LINE_MEMBER(vic20_state::write_user_joy2)
{
	m_user_joy2 = state;
}

WRITE_LINE_MEMBER(vic20_state::write_light_pen)
{
	m_light_pen = state;
	m_vic->lp_w(m_light_pen && m_user_light_pen);
}

WRITE_LINE_MEMBER(vic20_state::write_user_light_pen)
{
	m_user_light_pen = state;
	m_vic->lp_w(m_light_pen && m_user_light_pen);
}

WRITE_LINE_MEMBER(vic20_state::write_user_cassette_switch)
{
	m_user_cassette_switch = state;
}

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( vic20_common )
//-------------------------------------------------

static MACHINE_CONFIG_START( vic20, vic20_state )
	// devices
	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(READ8(vic20_state, via1_pa_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(vic20_state, via1_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(vic20_state, via1_pb_w))
	MCFG_VIA6522_CB1_HANDLER(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_b))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(PET_DATASSETTE_PORT_TAG, pet_datassette_port_device, motor_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(PET_USER_PORT_TAG, pet_user_port_device, write_m))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE(M6502_TAG, m6502_device, nmi_line))

	MCFG_DEVICE_ADD(M6522_2_TAG, VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(READ8(vic20_state, via2_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(vic20_state, via2_pb_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(vic20_state, via2_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(vic20_state, via2_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(vic20_state, via2_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE(M6502_TAG, m6502_device, irq_line))

	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, cbm_datassette_devices, "c1530", DEVWRITELINE(M6522_2_TAG, via6522_device, write_ca1))
	MCFG_CBM_IEC_ADD("c1541")
	MCFG_CBM_IEC_BUS_SRQ_CALLBACK(DEVWRITELINE(M6522_2_TAG, via6522_device, write_cb1))

	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, "joy")
	MCFG_VCS_CONTROL_PORT_TRIGGER_CALLBACK(WRITELINE(vic20_state, write_light_pen))

	MCFG_PET_USER_PORT_ADD(PET_USER_PORT_TAG, vic20_user_port_cards, NULL)
	MCFG_PET_USER_PORT_3_HANDLER(WRITELINE(vic20_state, exp_reset_w))
	MCFG_PET_USER_PORT_4_HANDLER(WRITELINE(vic20_state, write_user_joy0))
	MCFG_PET_USER_PORT_5_HANDLER(WRITELINE(vic20_state, write_user_joy1))
	MCFG_PET_USER_PORT_6_HANDLER(WRITELINE(vic20_state, write_user_joy2))
	MCFG_PET_USER_PORT_7_HANDLER(WRITELINE(vic20_state, write_user_light_pen))
	MCFG_PET_USER_PORT_8_HANDLER(WRITELINE(vic20_state, write_user_cassette_switch))
	MCFG_PET_USER_PORT_B_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_cb1))
	MCFG_PET_USER_PORT_C_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb0))
	MCFG_PET_USER_PORT_D_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb1))
	MCFG_PET_USER_PORT_E_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb2))
	MCFG_PET_USER_PORT_F_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb3))
	MCFG_PET_USER_PORT_H_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb4))
	MCFG_PET_USER_PORT_J_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb5))
	MCFG_PET_USER_PORT_K_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb6))
	MCFG_PET_USER_PORT_L_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_pb7))
	MCFG_PET_USER_PORT_M_HANDLER(DEVWRITELINE(M6522_1_TAG, via6522_device, write_cb2))

	MCFG_QUICKLOAD_ADD("quickload", vic20_state, cbm_vc20, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "vic1001_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "vic1001_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "vic1001_flop")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("5K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( ntsc, vic20 )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, MOS6560_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vic20_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks

	// video/sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_MOS6560_ADD(M6560_TAG, SCREEN_TAG, MOS6560_CLOCK, vic_videoram_map, vic_colorram_map)
	MCFG_MOS6560_POTX_CALLBACK(DEVREAD8(CONTROL1_TAG, vcs_control_port_device, pot_x_r))
	MCFG_MOS6560_POTY_CALLBACK(DEVREAD8(CONTROL1_TAG, vcs_control_port_device, pot_y_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_VIC20_EXPANSION_SLOT_ADD(VIC20_EXPANSION_SLOT_TAG, MOS6560_CLOCK, vic20_expansion_cards, NULL)
	MCFG_VIC20_EXPANSION_SLOT_IRQ_CALLBACK(INPUTLINE(M6502_TAG, M6502_IRQ_LINE))
	MCFG_VIC20_EXPANSION_SLOT_NMI_CALLBACK(INPUTLINE(M6502_TAG, M6502_NMI_LINE))
	MCFG_VIC20_EXPANSION_SLOT_RES_CALLBACK(WRITELINE(vic20_state, exp_reset_w))

	// software lists
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("cass_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "NTSC")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pal, vic20 )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, MOS6561_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vic20_mem)
	MCFG_M6502_DISABLE_DIRECT() // address decoding is 100% dynamic, no RAM/ROM banks

	// video/sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_MOS6561_ADD(M6560_TAG, SCREEN_TAG, MOS6561_CLOCK, vic_videoram_map, vic_colorram_map)
	MCFG_MOS6560_POTX_CALLBACK(DEVREAD8(CONTROL1_TAG, vcs_control_port_device, pot_x_r))
	MCFG_MOS6560_POTY_CALLBACK(DEVREAD8(CONTROL1_TAG, vcs_control_port_device, pot_y_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_VIC20_EXPANSION_SLOT_ADD(VIC20_EXPANSION_SLOT_TAG, MOS6561_CLOCK, vic20_expansion_cards, NULL)
	MCFG_VIC20_EXPANSION_SLOT_IRQ_CALLBACK(INPUTLINE(M6502_TAG, M6502_IRQ_LINE))
	MCFG_VIC20_EXPANSION_SLOT_NMI_CALLBACK(INPUTLINE(M6502_TAG, M6502_NMI_LINE))
	MCFG_VIC20_EXPANSION_SLOT_RES_CALLBACK(WRITELINE(vic20_state, exp_reset_w))

	// software lists
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("cass_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "PAL")
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  ROM( vic1001 )
//-------------------------------------------------

ROM_START( vic1001 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901486-02", 0x0000, 0x2000, CRC(336900d7) SHA1(c9ead45e6674d1042ca6199160e8583c23aeac22) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-02", 0x0000, 0x1000, CRC(fcfd8a4b) SHA1(dae61ac03065aa2904af5c123ce821855898c555) )
ROM_END


//-------------------------------------------------
//  ROM( vic20 )
//-------------------------------------------------

ROM_START( vic20 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS( 0, "cbm", "Original" )
	ROMX_LOAD( "901486-06.ue12", 0x0000, 0x2000, CRC(e5e7c174) SHA1(06de7ec017a5e78bd6746d89c2ecebb646efeb19), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS" )
	ROMX_LOAD( "jiffydos vic-20 ntsc.ue12", 0x0000, 0x2000, CRC(683a757f) SHA1(83fb83e97b5a840311dbf7e1fe56fe828f41936d), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-03.ud7", 0x0000, 0x1000, CRC(83e032a6) SHA1(4fd85ab6647ee2ac7ba40f729323f2472d35b9b4) )
ROM_END


//-------------------------------------------------
//  ROM( vic20p )
//-------------------------------------------------

ROM_START( vic20p )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS( 0, "cbm", "Original" )
	ROMX_LOAD( "901486-07.ue12", 0x0000, 0x2000, CRC(4be07cb4) SHA1(ce0137ed69f003a299f43538fa9eee27898e621e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS" )
	ROMX_LOAD( "jiffydos vic-20 pal.ue12", 0x0000, 0x2000, CRC(705e7810) SHA1(5a03623a4b855531b8bffd756f701306f128be2d), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-03.ud7", 0x0000, 0x1000, CRC(83e032a6) SHA1(4fd85ab6647ee2ac7ba40f729323f2472d35b9b4) )
ROM_END


//-------------------------------------------------
//  ROM( vic20_se )
//-------------------------------------------------

ROM_START( vic20_se )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "nec22081.206", 0x0000, 0x2000, CRC(b2a60662) SHA1(cb3e2f6e661ea7f567977751846ce9ad524651a3) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "nec22101.207", 0x0000, 0x1000, CRC(d808551d) SHA1(f403f0b0ce5922bd61bbd768bdd6f0b38e648c9f) )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE      INPUT      INIT                        COMPANY                             FULLNAME                    FLAGS
COMP( 1980, vic1001,    0,          0,      ntsc,       vic1001,    driver_device,  0,          "Commodore Business Machines",      "VIC-1001 (Japan)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1981, vic20,      vic1001,    0,      ntsc,       vic20,      driver_device,  0,          "Commodore Business Machines",      "VIC-20 (NTSC)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1981, vic20p,     vic1001,    0,      pal,        vic20,      driver_device,  0,          "Commodore Business Machines",      "VIC-20 / VC-20 (PAL)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1981, vic20_se,   vic1001,    0,      pal,        vic20s,     driver_device,  0,          "Commodore Business Machines",      "VIC-20 (Sweden/Finland)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
