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

    - clock is running too fast
    - access violation after OFF command
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
//  IRQ_CALLBACK( portfolio_int_ack )
//-------------------------------------------------

static IRQ_CALLBACK( portfolio_int_ack )
{
	portfolio_state *state = device->machine().driver_data<portfolio_state>();

	UINT8 vector = state->m_sivr;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(state->m_ip, i))
		{
			// clear interrupt pending bit
			state->m_ip &= ~(1 << i);

			if (i == 3)
				vector = state->m_sivr;
			else
				vector = INTERRUPT_VECTOR[i];

			break;
		}
	}

	state->check_interrupt();

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

	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7" };

	for (int row = 0; row < 8; row++)
	{
		UINT8 data = ioport(keynames[row])->read();

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

	speaker_level_w(m_speaker, !BIT(data, 7));

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
	data |= BIT(ioport("BATTERY")->read(), 0) << 6;

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
		UINT8 *rom = memregion(M80C88A_TAG)->base();
		program.install_rom(0xc0000, 0xdffff, rom);
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
//  AM_RANGE(0x8070, 0x8077) AM_DEVREADWRITE_LEGACY(M82C50A_TAG, ins8250_r, ins8250_w) // Serial Interface
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
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Atari") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('"') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Fn") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("PPI-PB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, strobe_w)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, autofeed_w)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, init_prime_w)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL) // PORT_WRITE_LINE_DEVICE(CENTRONICS_TAG, centronics_select_in_w)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PPI-PC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_READ_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, pe_r)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_READ_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, vcc_r)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_READ_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, fault_r)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_READ_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, busy_r)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_READ_LINE_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, ack_r)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

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

//-------------------------------------------------
//  PALETTE_INIT( portfolio )
//-------------------------------------------------

void portfolio_state::palette_init()
{
	palette_set_color(machine(), 0, MAKE_RGB(138, 146, 148));
	palette_set_color(machine(), 1, MAKE_RGB(92, 83, 88));
}

//-------------------------------------------------
//  HD61830_INTERFACE( lcdc_intf )
//-------------------------------------------------

static READ8_DEVICE_HANDLER( hd61830_rd_r )
{
	UINT16 address = ((offset & 0xff) << 3) | ((offset >> 12) & 0x07);
	UINT8 data = space.machine().root_device().memregion(HD61830_TAG)->base()[address];

	return data;
}

static HD61830_INTERFACE( lcdc_intf )
{
	SCREEN_TAG,
	DEVCB_HANDLER(hd61830_rd_r)
};

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
//  I8255_INTERFACE( ppi_intf )
//-------------------------------------------------

static I8255_INTERFACE( ppi_intf )
{
	DEVCB_NULL,													// Port A read
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),	// Port A write
	DEVCB_NULL,													// Port B read
	DEVCB_INPUT_PORT("PPI-PB"),									// Port B write
	DEVCB_INPUT_PORT("PPI-PC"),									// Port C read
	DEVCB_NULL													// Port C write
};

//-------------------------------------------------
//  ins8250_interface i8250_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( portfolio_state::i8250_intrpt_w )
{
	trigger_interrupt(INT_EXTERNAL);
}

static const ins8250_interface i8250_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(portfolio_state, i8250_intrpt_w),
	DEVCB_NULL,
	DEVCB_NULL
};

//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

static const centronics_interface centronics_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
};

//**************************************************************************
//  IMAGE LOADING
//**************************************************************************

//-------------------------------------------------
//  DEVICE_IMAGE_LOAD( portfolio_cart )
//-------------------------------------------------

static DEVICE_IMAGE_LOAD( portfolio_cart )
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

	/* set CPU interrupt vector callback */
	m_maincpu->set_irq_acknowledge_callback(portfolio_int_ack);

	/* memory expansions */
	switch (machine().device<ram_device>(RAM_TAG)->size())
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
		io.install_readwrite_handler(0x8070, 0x8077, read8_delegate(FUNC(ins8250_device::ins8250_r), (ins8250_device*)m_uart), write8_delegate(FUNC(ins8250_device::ins8250_w), (ins8250_device*)m_uart));
		break;

	case PID_PARALLEL:
		io.install_readwrite_handler(0x8078, 0x807b, read8_delegate(FUNC(i8255_device::read), (i8255_device*)m_ppi), write8_delegate(FUNC(i8255_device::write), (i8255_device*)m_ppi));
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

    /* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(72)
	MCFG_SCREEN_UPDATE_DEVICE(HD61830_TAG, hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(240, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 64-1)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_LENGTH(2)

	MCFG_GFXDECODE(portfolio)

	MCFG_HD61830_ADD(HD61830_TAG, XTAL_4_9152MHz/2/2, lcdc_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_I8255A_ADD(M82C55A_TAG, ppi_intf)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_INS8250_ADD(M82C50A_TAG, i8250_intf, XTAL_1_8432MHz) // should be MCFG_INS8250A_ADD
	MCFG_TIMER_DRIVER_ADD_PERIODIC("counter", portfolio_state, counter_tick, attotime::from_hz(XTAL_32_768kHz/16384))
	MCFG_TIMER_DRIVER_ADD_PERIODIC(TIMER_TICK_TAG, portfolio_state, system_tick, attotime::from_hz(XTAL_32_768kHz/32768))

	/* fake keyboard */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", portfolio_state, keyboard_tick, attotime::from_usec(2500))

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_INTERFACE("portfolio_cart")
	MCFG_CARTSLOT_LOAD(portfolio_cart)

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
COMP( 1989, pofo,	0,		0,		portfolio,	portfolio, driver_device,	0,		"Atari",	"Portfolio",	GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
