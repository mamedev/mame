// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Atari Portfolio

    http://portfolio.wz.cz/
    http://www.pofowiki.de/doku.php
    http://www.best-electronics-ca.com/portfoli.htm
    http://www.atari-portfolio.co.uk/pfnews/pf9.txt


    Undumped Atari cartridges:

    Utility-Card                HPC-701
    Finance-Card                HPC-702
    Science-Card                HPC-703
    File Manager / Tutorial     HPC-704
    PowerBASIC                  HPC-705
    Instant Spell               HPC-709
    Hyperlist                   HPC-713
    Bridge Baron                HPC-724
    Wine Companion              HPC-725
    Diet / Cholesterol Counter  HPC-726
    Astrologer                  HPC-728
    Stock Tracker               HPC-729
    Chess                       HPC-750


    Undumped 3rd party cartridges:

    Adcalc                      AAC-1000
    Alpha Paging Interface      SAMpage
    Business Contacts and Information Manager       BCIM
    Checkwriter
    Colossal Cave Adventure
    Drug Interactions
    Dynapulse                                       200M-A
    Form Letters
    FORTH programming system                        UTIL
    FX-3 DUAT Flight Software
    FX-4 Flight Planner
    Graphics Screens
    Marine Device Interface                         CM380 UMPIRE
    Message Mover (Mac)                             MSG-PKG6
    Message Mover (PC)                              MSG-PKG5
    Micro Hedge
    Micro-Roentgen Radiation Monitor                RM-60
    Patient Management
    PBase
    PDD2 Utilities
    Pharmaceuticals
    Physician's Reference I
    PIPELINE Fuel Management
    REACT
    Stocks Games
    Terminal+
    Timekeeper
    TIMEPAC-5

*/

/*

    TODO:

    - expansion port slot interface
    - clock is running too fast
    - create chargen ROM from tech manual
    - memory error interrupt vector
    - i/o port 8051
    - screen contrast
    - system tick frequency selection (1 or 128 Hz)
    - speaker
    - credit card memory (A:/B:)
    - software list

*/

#include "includes/portfoli.h"
#include "bus/rs232/rs232.h"
#include "rendlay.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	INT_TICK = 0,
	INT_KEYBOARD,
	INT_ERROR,
	INT_EXTERNAL
};

enum
{
	PID_COMMCARD = 0x00,
	PID_SERIAL,
	PID_PARALLEL,
	PID_PRINTER,
	PID_MODEM,
	PID_NONE = 0xff
};

static const UINT8 INTERRUPT_VECTOR[] = { 0x08, 0x09, 0x00 };



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupt - check interrupt status
//-------------------------------------------------

void portfolio_state::check_interrupt()
{
	int level = (m_ip & m_ie) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_INT0, level);
}


//-------------------------------------------------
//  trigger_interrupt - trigger interrupt request
//-------------------------------------------------

void portfolio_state::trigger_interrupt(int level)
{
	// set interrupt pending bit
	m_ip |= 1 << level;

	check_interrupt();
}


//-------------------------------------------------
//  irq_status_r - interrupt status read
//-------------------------------------------------

READ8_MEMBER( portfolio_state::irq_status_r )
{
	return m_ip;
}


//-------------------------------------------------
//  irq_mask_w - interrupt enable mask
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::irq_mask_w )
{
	m_ie = data;
	//logerror("IE %02x\n", data);

	check_interrupt();
}


//-------------------------------------------------
//  sivr_w - serial interrupt vector register
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::sivr_w )
{
	m_sivr = data;
	//logerror("SIVR %02x\n", data);
}


//-------------------------------------------------
//  IRQ_CALLBACK_MEMBER( portfolio_int_ack )
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(portfolio_state::portfolio_int_ack)
{
	UINT8 vector = m_sivr;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_ip, i))
		{
			// clear interrupt pending bit
			m_ip &= ~(1 << i);

			if (i == 3)
				vector = m_sivr;
			else
				vector = INTERRUPT_VECTOR[i];

			break;
		}
	}

	check_interrupt();

	return vector;
}



//**************************************************************************
//  KEYBOARD
//**************************************************************************

//-------------------------------------------------
//  scan_keyboard - scan keyboard
//-------------------------------------------------

void portfolio_state::scan_keyboard()
{
	UINT8 keycode = 0xff;

	UINT8 keydata[8] = { m_y0->read(), m_y1->read(), m_y2->read(), m_y3->read(),
							m_y4->read(), m_y5->read(), m_y6->read(), m_y7->read() };

	for (int row = 0; row < 8; row++)
	{
		UINT8 data = keydata[row];

		if (data != 0xff)
		{
			for (int col = 0; col < 8; col++)
			{
				if (!BIT(data, col))
				{
					keycode = (row * 8) + col;
				}
			}
		}
	}

	if (keycode != 0xff)
	{
		// key pressed
		if (keycode != m_keylatch)
		{
			m_keylatch = keycode;

			trigger_interrupt(INT_KEYBOARD);
		}
	}
	else
	{
		// key released
		if (m_keylatch != 0xff)
		{
			m_keylatch |= 0x80;

			trigger_interrupt(INT_KEYBOARD);
		}
	}
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( keyboard_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_state::keyboard_tick)
{
	scan_keyboard();
}


//-------------------------------------------------
//  keyboard_r - keyboard scan code register
//-------------------------------------------------

READ8_MEMBER( portfolio_state::keyboard_r )
{
	return m_keylatch;
}



//**************************************************************************
//  INTERNAL SPEAKER
//**************************************************************************

//-------------------------------------------------
//  speaker_w - internal speaker output
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::speaker_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       speaker level

	*/

	m_speaker->level_w(!BIT(data, 7));

	//logerror("SPEAKER %02x\n", data);
}



//**************************************************************************
//  POWER MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  power_w - power management
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::power_w )
{
	/*

	    bit     description

	    0
	    1       1=power off
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	//logerror("POWER %02x\n", data);
}


//-------------------------------------------------
//  battery_r - battery status
//-------------------------------------------------

READ8_MEMBER( portfolio_state::battery_r )
{
	/*

	    bit     signal      description

	    0
	    1
	    2
	    3
	    4
	    5       PDET        1=peripheral connected
	    6       BATD?       0=battery low
	    7                   1=cold boot

	*/

	UINT8 data = 0;

	/* peripheral detect */
	data |= (m_pid != PID_NONE) << 5;

	/* battery status */
	data |= BIT(m_battery->read(), 0) << 6;

	return data;
}


//-------------------------------------------------
//  unknown_w - ?
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::unknown_w )
{
	//logerror("UNKNOWN %02x\n", data);
}



//**************************************************************************
//  SYSTEM TIMERS
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( system_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_state::system_tick)
{
	trigger_interrupt(INT_TICK);
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( counter_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_state::counter_tick)
{
	m_counter++;
}


//-------------------------------------------------
//  counter_r - counter register read
//-------------------------------------------------

READ8_MEMBER( portfolio_state::counter_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0:
		data = m_counter & 0xff;
		break;

	case 1:
		data = m_counter >> 1;
		break;
	}

	return data;
}


//-------------------------------------------------
//  counter_w - counter register write
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::counter_w )
{
	switch (offset)
	{
	case 0:
		m_counter = (m_counter & 0xff00) | data;
		break;

	case 1:
		m_counter = (data << 8) | (m_counter & 0xff);
		break;
	}
}



//**************************************************************************
//  EXPANSION
//**************************************************************************

//-------------------------------------------------
//  ncc1_w - credit card memory select
//-------------------------------------------------

WRITE8_MEMBER( portfolio_state::ncc1_w )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (BIT(data, 0))
	{
		// system ROM
		program.install_rom(0xc0000, 0xdffff, m_rom);
	}
	else
	{
		// credit card memory
		program.unmap_readwrite(0xc0000, 0xdffff);
	}

	//logerror("NCC %02x\n", data);
}


//-------------------------------------------------
//  pid_r - peripheral identification
//-------------------------------------------------

READ8_MEMBER( portfolio_state::pid_r )
{
	/*

	    PID     peripheral

	    00      communication card
	    01      serial port
	    02      parallel port
	    03      printer peripheral
	    04      modem
	    05-3f   reserved
	    40-7f   user peripherals
	    80      file-transfer interface
	    81-ff   reserved

	*/

	return m_pid;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( portfolio_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( portfolio_mem, AS_PROGRAM, 8, portfolio_state )
	AM_RANGE(0x00000, 0x1efff) AM_RAM AM_SHARE("nvram1")
	AM_RANGE(0x1f000, 0x9efff) AM_RAM // expansion
	AM_RANGE(0xb0000, 0xb0fff) AM_MIRROR(0xf000) AM_RAM AM_SHARE("nvram2") // video RAM
	AM_RANGE(0xc0000, 0xdffff) AM_ROM AM_REGION(M80C88A_TAG, 0) // or credit card memory
	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION(M80C88A_TAG, 0x20000)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( portfolio_io )
//-------------------------------------------------

static ADDRESS_MAP_START( portfolio_io, AS_IO, 8, portfolio_state )
	AM_RANGE(0x8000, 0x8000) AM_READ(keyboard_r)
	AM_RANGE(0x8010, 0x8010) AM_DEVREADWRITE(HD61830_TAG, hd61830_device, data_r, data_w)
	AM_RANGE(0x8011, 0x8011) AM_DEVREADWRITE(HD61830_TAG, hd61830_device, status_r, control_w)
	AM_RANGE(0x8020, 0x8020) AM_WRITE(speaker_w)
	AM_RANGE(0x8030, 0x8030) AM_WRITE(power_w)
	AM_RANGE(0x8040, 0x8041) AM_READWRITE(counter_r, counter_w)
	AM_RANGE(0x8050, 0x8050) AM_READWRITE(irq_status_r, irq_mask_w)
	AM_RANGE(0x8051, 0x8051) AM_READWRITE(battery_r, unknown_w)
	AM_RANGE(0x8060, 0x8060) AM_RAM AM_SHARE("contrast")
//  AM_RANGE(0x8070, 0x8077) AM_DEVREADWRITE(M82C50A_TAG, ins8250_device, ins8250_r, ins8250_w) // Serial Interface
//  AM_RANGE(0x8078, 0x807b) AM_DEVREADWRITE(M82C55A_TAG, i8255_device, read, write) // Parallel Interface
	AM_RANGE(0x807c, 0x807c) AM_WRITE(ncc1_w)
	AM_RANGE(0x807f, 0x807f) AM_READWRITE(pid_r, sivr_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( portfolio )
//-------------------------------------------------

static INPUT_PORTS_START( portfolio )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Atari") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('"') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Fn") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x00, "Low Battery" )

	PORT_START("PERIPHERAL")
	PORT_CONFNAME( 0xff, PID_NONE, "Peripheral" )
	PORT_CONFSETTING( PID_NONE, DEF_STR( None ) )
	PORT_CONFSETTING( PID_PARALLEL, "Intelligent Parallel Interface (HPC-101)" )
	PORT_CONFSETTING( PID_SERIAL, "Serial Interface (HPC-102)" )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

PALETTE_INIT_MEMBER(portfolio_state, portfolio)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


//-------------------------------------------------
//  HD61830_INTERFACE( lcdc_intf )
//-------------------------------------------------

READ8_MEMBER( portfolio_state::hd61830_rd_r )
{
	UINT16 address = ((offset & 0xff) << 3) | ((offset >> 12) & 0x07);
	UINT8 data = m_char_rom[address];

	return data;
}


//-------------------------------------------------
//  gfx_layout charlayout
//-------------------------------------------------

static const gfx_layout charlayout =
{
	6, 8,
	256,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2 },
	{ STEP8(0,8) },
	8*8
};


//-------------------------------------------------
//  GFXDECODE( portfolio )
//-------------------------------------------------

static GFXDECODE_START( portfolio )
	GFXDECODE_ENTRY( HD61830_TAG, 0, charlayout, 0, 2 )
GFXDECODE_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ins8250_interface i8250_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( portfolio_state::i8250_intrpt_w )
{
	if (state)
		trigger_interrupt(INT_EXTERNAL);
}

//**************************************************************************
//  IMAGE LOADING
//**************************************************************************

//-------------------------------------------------
//  DEVICE_IMAGE_LOAD( portfolio_cart )
//-------------------------------------------------

DEVICE_IMAGE_LOAD_MEMBER( portfolio_state, portfolio_cart )
{
	return IMAGE_INIT_FAIL;
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void portfolio_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* memory expansions */
	switch (m_ram->size())
	{
	case 128 * 1024:
		program.unmap_readwrite(0x1f000, 0x9efff);
		break;

	case 384 * 1024:
		program.unmap_readwrite(0x5f000, 0x9efff);
		break;
	}

	/* set initial values */
	m_keylatch = 0xff;
	m_sivr = 0x2a;
	m_pid = 0xff;

	/* register for state saving */
	save_item(NAME(m_ip));
	save_item(NAME(m_ie));
	save_item(NAME(m_sivr));
	save_item(NAME(m_counter));
	save_item(NAME(m_keylatch));
	save_pointer(NAME(m_contrast.target()), m_contrast.bytes());
	save_item(NAME(m_pid));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void portfolio_state::machine_reset()
{
	address_space &io = m_maincpu->space(AS_IO);

	// peripherals
	m_pid = ioport("PERIPHERAL")->read();

	io.unmap_readwrite(0x8070, 0x807b);
	io.unmap_readwrite(0x807d, 0x807e);

	switch (m_pid)
	{
	case PID_SERIAL:
		io.install_readwrite_handler(0x8070, 0x8077, READ8_DEVICE_DELEGATE(m_uart, ins8250_device, ins8250_r), WRITE8_DEVICE_DELEGATE(m_uart, ins8250_device, ins8250_w));
		break;

	case PID_PARALLEL:
		io.install_readwrite_handler(0x8078, 0x807b, READ8_DEVICE_DELEGATE(m_ppi, i8255_device, read), WRITE8_DEVICE_DELEGATE(m_ppi, i8255_device, write));
		break;
	}
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( portfolio )
//-------------------------------------------------

static MACHINE_CONFIG_START( portfolio, portfolio_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(M80C88A_TAG, I8088, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(portfolio_mem)
	MCFG_CPU_IO_MAP(portfolio_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(portfolio_state,portfolio_int_ack)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(72)
	MCFG_SCREEN_UPDATE_DEVICE(HD61830_TAG, hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(240, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 64-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(portfolio_state, portfolio)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", portfolio)

	MCFG_DEVICE_ADD(HD61830_TAG, HD61830, XTAL_4_9152MHz/2/2)
	MCFG_HD61830_RD_CALLBACK(READ8(portfolio_state, hd61830_rd_r))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(M82C55A_TAG, I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITE8("cent_ctrl_out", output_latch_device, write))
	MCFG_I8255_IN_PORTC_CB(DEVREAD8("cent_status_in", input_buffer_device, read))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit5))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit4))
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit3))
	MCFG_CENTRONICS_SELECT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit1))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit0))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)
	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_DEVICE_ADD("cent_ctrl_out", OUTPUT_LATCH, 0)
	MCFG_OUTPUT_LATCH_BIT0_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))
	MCFG_OUTPUT_LATCH_BIT1_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_autofd))
	MCFG_OUTPUT_LATCH_BIT2_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_init))
	MCFG_OUTPUT_LATCH_BIT3_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_select_in))

	MCFG_DEVICE_ADD(M82C50A_TAG, INS8250, XTAL_1_8432MHz) // should be INS8250A
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(portfolio_state, i8250_intrpt_w))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("counter", portfolio_state, counter_tick, attotime::from_hz(XTAL_32_768kHz/16384))
	MCFG_TIMER_DRIVER_ADD_PERIODIC(TIMER_TICK_TAG, portfolio_state, system_tick, attotime::from_hz(XTAL_32_768kHz/32768))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(M82C50A_TAG, ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(M82C50A_TAG, ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(M82C50A_TAG, ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE(M82C50A_TAG, ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(M82C50A_TAG, ins8250_uart_device, cts_w))

	/* fake keyboard */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", portfolio_state, keyboard_tick, attotime::from_usec(2500))

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "portfolio_cart")
	MCFG_GENERIC_LOAD(portfolio_state, portfolio_cart)

	/* memory card */
/*  MCFG_MEMCARD_ADD("memcard_a")
    MCFG_MEMCARD_EXTENSION_LIST("bin")
    MCFG_MEMCARD_LOAD(portfolio_memcard)
    MCFG_MEMCARD_SIZE_OPTIONS("32K,64K,128K")

    MCFG_MEMCARD_ADD("memcard_b")
    MCFG_MEMCARD_EXTENSION_LIST("bin")
    MCFG_MEMCARD_LOAD(portfolio_memcard)
    MCFG_MEMCARD_SIZE_OPTIONS("32K,64K,128K")*/

	/* software lists */
//  MCFG_SOFTWARE_LIST_ADD("cart_list", "pofo")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("384K,640K")

	MCFG_NVRAM_ADD_RANDOM_FILL("nvram1")
	MCFG_NVRAM_ADD_RANDOM_FILL("nvram2")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pofo )
//-------------------------------------------------

ROM_START( pofo )
	ROM_REGION( 0x40000, M80C88A_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "dip1072", "DIP DOS 1.072" )
	ROMX_LOAD( "rom b.u4", 0x00000, 0x20000, BAD_DUMP CRC(c9852766) SHA1(c74430281bc717bd36fd9b5baec1cc0f4489fe82), ROM_BIOS(1) ) // dumped with debug.com
	ROMX_LOAD( "rom a.u3", 0x20000, 0x20000, BAD_DUMP CRC(b8fb730d) SHA1(1b9d82b824cab830256d34912a643a7d048cd401), ROM_BIOS(1) ) // dumped with debug.com

	ROM_REGION( 0x800, HD61830_TAG, 0 )
	ROM_LOAD( "hd61830 external character generator", 0x000, 0x800, BAD_DUMP CRC(747a1db3) SHA1(a4b29678fdb43791a8ce4c1ec778f3231bb422c5) ) // typed in from manual
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY     FULLNAME        FLAGS */
COMP( 1989, pofo,   0,      0,      portfolio,  portfolio, driver_device,   0,      "Atari",    "Portfolio",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
