/***************************************************************************

    Canon Cat driver by Miodrag Milanovic

    12/06/2009 Skeleton driver.
    15/06/2009 Working driver

Pictures: http://www.regnirps.com/Apple6502stuff/apple_iie_cat.htm

How to enable the FORTH interpreter (http://canoncat.org/canoncat/enableforth.html)

The definitive instructions of going Forth on a Cat, tested on a living Cat.
The Canon Cat is programmed with FORTH, a remarkably different programming language in itself.
On the Canon Cat, you can access the FORTH interpreter with a special sequence of key presses.
Here are exact working instructions thanks to Sandy Bumgarner:
- Type Enable Forth Language exactly like that, capitals and all.
- Highlight from the En to the ge, exactly [i.e. Leap back to Enable by keeping the left Leap
  key down while typing E n a, release the leap key, then click both leap keys simultaneously],
  and press USE FRONT with ERASE (the ANSWER command).
- The Cat should beep and flash the ruler.
- Then press USE FRONT and the SHIFT keys together and tap the space bar. Note that the cursor
  stops blinking. Now a Pressing the RETURN key gets the Forth OK and you are 'in' as they say.


ToDo:
- Floppy drive (3.5", 256kb)
- Beeper/speaker
- Swyft - figure out the keyboard
- Centronics port
- RS232C port

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68681.h"
#include "machine/nvram.h"

class cat_state : public driver_device
{
public:
	cat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_p_sram(*this, "p_sram"),
		m_p_videoram(*this, "p_videoram"){ }

	DECLARE_WRITE16_MEMBER(cat_video_status_w);
	DECLARE_WRITE16_MEMBER(cat_test_mode_w);
	DECLARE_READ16_MEMBER(cat_modem_r);
	DECLARE_WRITE16_MEMBER(cat_modem_w);
	DECLARE_READ16_MEMBER(cat_battery_r);
	DECLARE_WRITE16_MEMBER(cat_printer_w);
	DECLARE_READ16_MEMBER(cat_floppy_r);
	DECLARE_WRITE16_MEMBER(cat_floppy_w);
	DECLARE_READ16_MEMBER(cat_keyboard_r);
	DECLARE_WRITE16_MEMBER(cat_keyboard_w);
	DECLARE_WRITE16_MEMBER(cat_video_w);
	DECLARE_READ16_MEMBER(cat_something_r);
	optional_shared_ptr<UINT16> m_p_sram;
	required_shared_ptr<UINT16> m_p_videoram;
	UINT8 m_duart_inp;// = 0x0e;
	UINT8 m_video_enable;
	UINT16 m_pr_cont;
	UINT8 m_keyboard_line;
	emu_timer *m_keyboard_timer;
};

WRITE16_MEMBER( cat_state::cat_video_status_w )
{
	m_video_enable = BIT( data, 3 );
}

WRITE16_MEMBER( cat_state::cat_test_mode_w )
{
}

READ16_MEMBER( cat_state::cat_modem_r )
{
	return 0;
}

WRITE16_MEMBER( cat_state::cat_modem_w )
{
}

READ16_MEMBER( cat_state::cat_battery_r )
{
	/* just return that battery is full */
	return 0x7fff;
}

WRITE16_MEMBER( cat_state::cat_printer_w )
{
	m_pr_cont = data;
}

READ16_MEMBER( cat_state::cat_floppy_r )
{
	return 0;
}

WRITE16_MEMBER( cat_state::cat_floppy_w )
{
}

READ16_MEMBER( cat_state::cat_keyboard_r )
{
	UINT16 retVal = 0;
	// Read country code
	if (m_pr_cont == 0x0900)
		retVal = ioport("DIPSW1")->read();

	// Regular keyboard read
	if (m_pr_cont == 0x0800 || m_pr_cont == 0x0a00)
	{
		retVal=0xff00;
		switch(m_keyboard_line)
		{
			case 0x01: retVal = ioport("LINE0")->read() << 8; break;
			case 0x02: retVal = ioport("LINE1")->read() << 8; break;
			case 0x04: retVal = ioport("LINE2")->read() << 8; break;
			case 0x08: retVal = ioport("LINE3")->read() << 8; break;
			case 0x10: retVal = ioport("LINE4")->read() << 8; break;
			case 0x20: retVal = ioport("LINE5")->read() << 8; break;
			case 0x40: retVal = ioport("LINE6")->read() << 8; break;
			case 0x80: retVal = ioport("LINE7")->read() << 8; break;
		}
	}
	return retVal;
}

WRITE16_MEMBER( cat_state::cat_keyboard_w )
{
	m_keyboard_line = data >> 8;
}

WRITE16_MEMBER( cat_state::cat_video_w )
{
/*
 006500AE ,          ( HSS HSync Strart    89 )
 006480C2 ,          ( HST End HSync   96 )
 006400CE ,          ( HSE End H Line    104 )
 006180B0 ,          ( VDE Active Lines    344 )
 006100D4 ,          ( VSS VSync Start   362 )
 006080F4 ,          ( VST End of VSync    378 )
 00600120 ,          ( VSE End of Frame    400 )
 006581C0 ,          ( VOC Video Control Normal Syncs )
 */
}

READ16_MEMBER( cat_state::cat_something_r )
{
	return 0x00ff;
}

static ADDRESS_MAP_START(cat_mem, AS_PROGRAM, 16, cat_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0003ffff) AM_ROM // 256 KB ROM
	AM_RANGE(0x00040000, 0x00043fff) AM_RAM AM_SHARE("p_sram") // SRAM powered by battery
	AM_RANGE(0x00200000, 0x0027ffff) AM_ROM AM_REGION("svrom",0x0000) // SV ROM
	AM_RANGE(0x00400000, 0x0047ffff) AM_RAM AM_SHARE("p_videoram") // 512 KB RAM
	AM_RANGE(0x00600000, 0x0065ffff) AM_WRITE(cat_video_w) // Video chip
	AM_RANGE(0x00800000, 0x00800001) AM_READWRITE(cat_floppy_r, cat_floppy_w)
	AM_RANGE(0x00800002, 0x00800003) AM_WRITE(cat_keyboard_w)
	AM_RANGE(0x00800008, 0x00800009) AM_READ(cat_something_r)
	AM_RANGE(0x0080000a, 0x0080000b) AM_READ(cat_keyboard_r)
	AM_RANGE(0x0080000e, 0x0080000f) AM_READWRITE(cat_battery_r,cat_printer_w)
	AM_RANGE(0x00810000, 0x0081001f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0x00820000, 0x008200ff) AM_READWRITE(cat_modem_r, cat_modem_w)// modem
	AM_RANGE(0x00840000, 0x00840001) AM_WRITE(cat_video_status_w) // Video status
	AM_RANGE(0x00860000, 0x00860001) AM_WRITE(cat_test_mode_w) // Test mode
ADDRESS_MAP_END

static ADDRESS_MAP_START(swyft_mem, AS_PROGRAM, 16, cat_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0000ffff) AM_ROM // 64 KB ROM
	AM_RANGE(0x00040000, 0x000fffff) AM_RAM AM_SHARE("p_videoram")
ADDRESS_MAP_END

/* Input ports */

/* 2009-07 FP
   FIXME: Natural keyboard does not catch all the Shifted chars. No idea of the reason!  */
static INPUT_PORTS_START( cat )
	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x8000, 0x8000, "Mode" )
	PORT_DIPSETTING(	0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x0000, "Diagnostic" )
	PORT_DIPNAME( 0x7f00,0x7f00, "Country code" )
	PORT_DIPSETTING(	0x7f00, "United States" )
	PORT_DIPSETTING(	0x7e00, "Canada" )
	PORT_DIPSETTING(	0x7d00, "United Kingdom" )
	PORT_DIPSETTING(	0x7c00, "Norway" )
	PORT_DIPSETTING(	0x7b00, "France" )
	PORT_DIPSETTING(	0x7a00, "Denmark" )
	PORT_DIPSETTING(	0x7900, "Sweden" )
	PORT_DIPSETTING(	0x7800, DEF_STR(Japan) )
	PORT_DIPSETTING(	0x7700, "West Germany" )
	PORT_DIPSETTING(	0x7600, "Netherlands" )
	PORT_DIPSETTING(	0x7500, "Spain" )
	PORT_DIPSETTING(	0x7400, "Italy" )
	PORT_DIPSETTING(	0x7300, "Latin America" )
	PORT_DIPSETTING(	0x7200, "South Africa" )
	PORT_DIPSETTING(	0x7100, "Switzerland" )
	PORT_DIPSETTING(	0x7000, "ASCII" )

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('\xa2')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('n') PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left USE FRONT") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right USE FRONT") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('\xbd') PORT_CHAR('\xbc')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left LEAP") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Leap") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Page") PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UNDO") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\xb1') PORT_CHAR('\xb0')
INPUT_PORTS_END

static INPUT_PORTS_START( swyft )
INPUT_PORTS_END


static TIMER_CALLBACK(keyboard_callback)
{
	machine.device("maincpu")->execute().set_input_line(M68K_IRQ_1, ASSERT_LINE);
}

static IRQ_CALLBACK(cat_int_ack)
{
	device->machine().device("maincpu")->execute().set_input_line(M68K_IRQ_1,CLEAR_LINE);
	return M68K_INT_ACK_AUTOVECTOR;
}

static MACHINE_START(cat)
{
	cat_state *state = machine.driver_data<cat_state>();

	state->m_duart_inp = 0x0e;
	state->m_keyboard_timer = machine.scheduler().timer_alloc(FUNC(keyboard_callback));
	machine.device<nvram_device>("nvram")->set_base(state->m_p_sram, 0x4000);
}

static MACHINE_RESET(cat)
{
	cat_state *state = machine.driver_data<cat_state>();
	device_set_irq_callback(machine.device("maincpu"), cat_int_ack);
	state->m_keyboard_timer->adjust(attotime::zero, 0, attotime::from_hz(120));
}

static VIDEO_START( cat )
{
}

static SCREEN_UPDATE_IND16( cat )
{
	cat_state *state = screen.machine().driver_data<cat_state>();
	UINT16 code;
	int y, x, b;

	int addr = 0;
	if (state->m_video_enable == 1)
	{
		for (y = 0; y < 344; y++)
		{
			int horpos = 0;
			for (x = 0; x < 42; x++)
			{
				code = state->m_p_videoram[addr++];
				for (b = 15; b >= 0; b--)
				{
					bitmap.pix16(y, horpos++) = (code >> b) & 0x01;
				}
			}
		}
	} else {
		const rectangle black_area(0, 672 - 1, 0, 344 - 1);
		bitmap.fill(0, black_area);
	}
	return 0;
}

static TIMER_CALLBACK( swyft_reset )
{
	memset(machine.device("maincpu")->memory().space(AS_PROGRAM)->get_read_ptr(0xe2341), 0xff, 1);
}

static MACHINE_START(swyft)
{
}

static MACHINE_RESET(swyft)
{
	machine.scheduler().timer_set(attotime::from_usec(10), FUNC(swyft_reset));
}

static VIDEO_START( swyft )
{
}

static SCREEN_UPDATE_IND16( swyft )
{
	cat_state *state = screen.machine().driver_data<cat_state>();
	UINT16 code;
	int y, x, b;

	int addr = 0;
	for (y = 0; y < 242; y++)
	{
		int horpos = 0;
		for (x = 0; x < 20; x++)
		{
			code = state->m_p_videoram[addr++];
			for (b = 15; b >= 0; b--)
			{
				bitmap.pix16(y, horpos++) = (code >> b) & 0x01;
			}
		}
	}
	return 0;
}

static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	//logerror("duart_irq_handler\n");
}

static void duart_tx(device_t *device, int channel, UINT8 data)
{
}

static UINT8 duart_input(device_t *device)
{
	cat_state *state = device->machine().driver_data<cat_state>();

	if (state->m_duart_inp != 0)
	{
		state->m_duart_inp = 0;
		return 0x0e;
	}
	else
	{
		state->m_duart_inp = 0x0e;
		return 0x00;
	}
}

static const duart68681_config cat_duart68681_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	NULL
};

static MACHINE_CONFIG_START( cat, cat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_5MHz)
	MCFG_CPU_PROGRAM_MAP(cat_mem)

	MCFG_MACHINE_START(cat)
	MCFG_MACHINE_RESET(cat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(672, 344)
	MCFG_SCREEN_VISIBLE_AREA(0, 672-1, 0, 344-1)
	MCFG_SCREEN_UPDATE_STATIC(cat)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_VIDEO_START(cat)

	MCFG_DUART68681_ADD( "duart68681", XTAL_5MHz, cat_duart68681_config )

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( swyft, cat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_5MHz)
	MCFG_CPU_PROGRAM_MAP(swyft_mem)

	MCFG_MACHINE_START(swyft)
	MCFG_MACHINE_RESET(swyft)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 242)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 242-1)
	MCFG_SCREEN_UPDATE_STATIC(swyft)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_VIDEO_START(swyft)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( swyft )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "infoapp.lo", 0x0000, 0x8000, CRC(52c1bd66) SHA1(b3266d72970f9d64d94d405965b694f5dcb23bca) )
	ROM_LOAD( "infoapp.hi", 0x8000, 0x8000, CRC(83505015) SHA1(693c914819dd171114a8c408f399b56b470f6be0) )
ROM_END

ROM_START( cat )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	// SYS ROM
	ROM_LOAD16_BYTE( "r240l0.bin", 0x00001, 0x10000, CRC(1b89bdc4) SHA1(39c639587dc30f9d6636b46d0465f06272838432) )
	ROM_LOAD16_BYTE( "r240h0.bin", 0x00000, 0x10000, CRC(94f89b8c) SHA1(6c336bc30636a02c625d31f3057ec86bf4d155fc) )
	ROM_LOAD16_BYTE( "r240l1.bin", 0x20001, 0x10000, CRC(1a73be4f) SHA1(e2de2cb485f78963368fb8ceba8fb66ca56dba34) )
	ROM_LOAD16_BYTE( "r240h1.bin", 0x20000, 0x10000, CRC(898dd9f6) SHA1(93e791dd4ed7e4afa47a04df6fdde359e41c2075) )

	ROM_REGION( 0x80000, "svrom", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1985, swyft,0,       0,      swyft,    swyft, driver_device,    0,   "Information Applicance Inc", "Swyft", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1987, cat,  swyft,   0,      cat,      cat, driver_device,      0,   "Canon",   "Cat", GAME_NOT_WORKING | GAME_NO_SOUND)
