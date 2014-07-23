// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Texas Instruments TI-74 BASICALC
  hardware family: CC-40 -> TI-74 BASICALC -> TI-95 PROCALC

          DOCK-BUS
        --||||||||---
    C  ==           |
    a  ==           |
    r  ==  HN61256  |
    t  ==           ----------------------------
        |                                      |
  -------            C70009          4MHz      |
  |        HM6264                    RC4193N   |
  |                                            |
  |                                            |
  |                                            |
  |                                            |
  ---------------||||||||||||||||||||||||-------
                 ||||||||||||||||||||||||
  ---------------||||||||||||||||||||||||-------
  |              *HD44100H   *HD44780A00       |
  |                                            |
  |                                            |
  |                                            |
  |                                            |
  ----------                                   |
           |-----------------------------------|
           ||                                 ||
           || LCD 1 line, 31 chars + 14 indic.||
           ||                                 ||
           |-----------------------------------|
           -------------------------------------

  IC1 HN61256PC93 - Hitachi DIP-28 32KB CMOS Mask PROM
  IC2 C70009      - Texas Instruments TMS70C40 with some TI custom I/O mods, 54 pins (also seen labeled TMS70C46)
                    running at max 4MHz. 128 bytes internal RAM, 4KB internal ROM
  IC3 HM6264LP-15 - Hitachi 8KB SRAM (battery backed)
  RC4193N         - Micropower Switching Regulator
  HD44100H        - 60-pin QFP Hitachi HD44100 LCD Driver
  HD44780A00      - 80-pin TFP Hitachi HD44780 LCD Controller

  *               - indicates that it's on the other side of the PCB

  
  Overall, the hardware is very similar to TI CC-40. A lot has been shuffled around
  to cut down on complexity (and probably for protection too).
  
  TI-74 is powered by 4 AAA batteries. These will also save internal RAM,
  provided that the machine is turned off properly.
  
  
  TODO:
  - control_r/w clock divider (currently always running full speed)
  - external ram cartridge
  - DOCK-BUS interface and peripherals
    * CI-7 cassette interface
    * PC-324 thermal printer
    (+ old Hexbus devices can be connected via a converter cable)

***************************************************************************/

#include "emu.h"
#include "cpu/tms7000/tms7000.h"
#include "video/hd44780.h"
#include "machine/nvram.h"
#include "imagedev/cartslot.h"

#include "ti74.lh"


class ti74_state : public driver_device
{
public:
	ti74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<tms70c46_device> m_maincpu;

	ioport_port *m_key_matrix[8];
	emu_timer *m_poweron_timer;

	UINT8 m_control;
	UINT8 m_key_select;
	UINT16 m_ext_address;
	UINT8 m_power;

	void update_lcd_indicator(UINT8 y, UINT8 x, int state);

	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(ext_address_w);

	virtual void machine_reset();
	virtual void machine_start();
	DECLARE_PALETTE_INIT(ti74);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(ti74_cartridge);
	TIMER_CALLBACK_MEMBER(poweron_timer_cb);
};



/***************************************************************************

  File Handling

***************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(ti74_state, ti74_cartridge)
{
	UINT8* pos = memregion("user1")->base();
	offs_t size;

	if (image.software_entry() == NULL)
		size = image.length();
	else
		size = image.get_software_region_length("rom");

	// max size is 32KB
	if (size > 0x8000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return IMAGE_INIT_FAIL;
	}

	if (image.software_entry() == NULL)
	{
		if (image.fread(pos, size) != size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read file");
			return IMAGE_INIT_FAIL;
		}
	}
	else
		memcpy(pos, image.get_software_region("rom"), size);

	return IMAGE_INIT_PASS;
}



/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(ti74_state, ti74)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void ti74_state::update_lcd_indicator(UINT8 y, UINT8 x, int state)
{
	// reference _________________...
	// output#  |10     11     12     13     14      2      3      4
	// above    | <    SHIFT   CTL    FN     I/O    UCL    _LOW    >
	// ---- raw lcd screen here ----
	// under    |      BASIC   CALC   DEG    RAD    GRAD   STAT
	// output#  |       63     64      1     62     53     54
	output_set_lamp_value(y * 10 + x, state);
}

static HD44780_PIXEL_UPDATE(ti74_pixel_update)
{
	if (line == 1 && pos == 15)
	{
		// the last char is used to control lcd indicators
		ti74_state *driver_state = device.machine().driver_data<ti74_state>();
		driver_state->update_lcd_indicator(y, x, state);
	}
	else if (line < 2 && pos < 16)
	{
		// internal: 2*16, external: 1*31 + indicators
		if (y == 7) y++; // the cursor is slightly below the 5x7 character
		bitmap.pix16(1 + y, 1 + line*16*6 + pos*6 + x) = state;
	}
}



/***************************************************************************

  I/O, Memory Maps

***************************************************************************/

READ8_MEMBER(ti74_state::control_r)
{
	return m_control;
}

WRITE8_MEMBER(ti74_state::control_w)
{
	// ? clock divider related
	m_control = data;
}

READ8_MEMBER(ti74_state::keyboard_r)
{
	UINT8 ret = 0;

	// read selected keyboard rows
	for (int i = 0; i < 8; i++)
	{
		if (m_key_select >> i & 1)
			ret |= m_key_matrix[i]->read();
	}
	
	return ret;
}

WRITE8_MEMBER(ti74_state::keyboard_w)
{
	// d(0-7): select keyboard column
	m_key_select = data;
}

WRITE8_MEMBER(ti74_state::bankswitch_w)
{
	// d0-d1: system rom bankswitch
	membank("sysbank")->set_entry(data & 3);
	
	// d2: power-on latch
	if (~data & 4 && m_power)
	{
		m_power = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // stop running
	}
	
	// other bits: N/C
}

WRITE8_MEMBER(ti74_state::ext_address_w)
{
	// set external memory addressbus (DOCK-BUS related)
	if (offset)
		m_ext_address = (m_ext_address & 0xff00) | data;
	else
		m_ext_address = (m_ext_address & 0x00ff) | data << 8;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, ti74_state )
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x010c, 0x010c) AM_WRITE(keyboard_w) AM_READNOP
	AM_RANGE(0x010d, 0x010d) AM_NOP // ? DOCK-BUS related
	AM_RANGE(0x010e, 0x010e) AM_NOP // ? DOCK-BUS related
	AM_RANGE(0x010f, 0x010f) AM_NOP // ? DOCK-BUS related
	AM_RANGE(0x0118, 0x0118) AM_READWRITE(control_r, control_w)

	AM_RANGE(0x1000, 0x1001) AM_DEVREADWRITE("hd44780", hd44780_device, read, write)

	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("6264.ic3")
	AM_RANGE(0x4000, 0xbfff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("sysbank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, ti74_state )
	AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_READ(keyboard_r)
	AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_WRITE(bankswitch_w)
	AM_RANGE(TMS7000_PORTC, TMS7000_PORTD) AM_WRITE(ext_address_w)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( ti74 )
	// 8x8 keyboard matrix, RESET and ON buttons are not on it
	// Unused entries are not connected, but some might have a purpose for factory testing
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("LEFT")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("RIGHT")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR('\'') PORT_NAME("SPACE  '")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('(') PORT_NAME("UP  (")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_CHAR('=') PORT_NAME("ENTER  =")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("CLR  UCL")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(')') PORT_NAME("DOWN  )")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("RUN")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('~') PORT_CHAR('?') PORT_NAME("+/-  ?")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_NAME("7  DEL")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_NAME("BREAK")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("8  INS")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("MODE")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_NAME("9  PB")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("OFF")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("*")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("/")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_NAME("FN")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("CTL")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

TIMER_CALLBACK_MEMBER(ti74_state::poweron_timer_cb)
{
	m_power = 1;
	
	// battery ok/low status is on int1 line!
	m_maincpu->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
}

void ti74_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	bankswitch_w(space, 0, 0);
	
	// give the system some time to boot before switching poweron latch
	m_power = 0;
	m_maincpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
	m_poweron_timer->adjust(attotime::from_msec(10));
}

void ti74_state::machine_start()
{
	static const char *const tags[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7" };
	for (int i = 0; i < 8; i++)
		m_key_matrix[i] = ioport(tags[i]);

	membank("sysbank")->configure_entries(0, 4, memregion("system")->base(), 0x2000);

	m_poweron_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti74_state::poweron_timer_cb), this));
	m_poweron_timer->adjust(attotime::never);

	// zerofill
	m_control = 0;
	m_key_select = 0;
	m_ext_address = 0;
	m_power = 0;

	// register for savestates
	save_item(NAME(m_control));
	save_item(NAME(m_key_select));
	save_item(NAME(m_ext_address));
	save_item(NAME(m_power));
}

static MACHINE_CONFIG_START( ti74, ti74_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS70C46, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_NVRAM_ADD_0FILL("6264.ic3")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*31+1, 9*1+1+1)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*31, 0, 9*1+1)
	MCFG_DEFAULT_LAYOUT(layout_ti74)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(ti74_state, ti74)

	MCFG_HD44780_ADD("hd44780") // 270kHz
	MCFG_HD44780_LCD_SIZE(2, 16) // internal: 2*16, external: 1*31 + indicators
	MCFG_HD44780_PIXEL_UPDATE_CB(ti74_pixel_update)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(ti74_state, ti74_cartridge)
	MCFG_CARTSLOT_INTERFACE("ti74_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "ti74_cart")
MACHINE_CONFIG_END



/***************************************************************************

  ROM Definitions

***************************************************************************/

ROM_START( ti74 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tms70c46.ic2", 0xf000, 0x1000, CRC(55a2f7c0) SHA1(530e3de42f2e304c8f4805ad389f38a459ec4e33) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "001060281-1.ic1", 0x0000, 0x8000, CRC(019aaa2f) SHA1(04a1e694a49d50602e45a7834846de4d9f7d587d) ) // system rom, banked

	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF ) // cartridge area
ROM_END


COMP( 1985, ti74, 0, 0, ti74, ti74, driver_device, 0, "Texas Instruments", "TI-74 BASICALC", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
