// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/******************************************************************************
 *  Sharp MZ700
 *
 *  Reference: https://original.sharpmz.org/
 *
 *  MZ700 memory map
 *
 *  0000-0FFF   1Z-013A ROM or RAM
 *  1000-CFFF   RAM
 *  D000-D7FF   videoram or RAM
 *  D800-DFFF   colorram or RAM
 *  E000-FFFF   memory mapped IO or RAM
 *
 *      xxx0    PPI8255 port A (output)
 *              bit 7   556RST (reset NE556)
 *              bit 6-4 unused
 *              bit 3-0 keyboard row demux (LS145)
 *
 *      xxx1    PPI8255 port B (input)
 *              bit 7-0 keyboard matrix code
 *
 *      xxx2    PPI8255 port C (input/output)
 *              bit 7 R -VBLANK input
 *              bit 6 R 556OUT (1.5Hz)
 *              bit 5 R RDATA from cassette
 *              bit 4 R MOTOR from cassette
 *              bit 3 W M-ON control
 *              bit 2 W INTMASK 1=enable 0=disabel clock interrupt
 *              bit 1 W WDATA to cassette
 *              bit 0 W unused
 *
 *      xxx3    PPI8255 control
 *
 *      xxx4    PIT8253 timer 0 (clock input 1,108800 MHz)
 *      xxx5    PIT8253 timer 1 (clock input 15,611 kHz)
 *      xxx6    PIT8253 timer 2 (clock input OUT1 1Hz (default))
 *      xxx7    PIT8253 control/status
 *
 *      xxx8    bit 7 R -HBLANK
 *              bit 6 R unused
 *              bit 5 R unused
 *              bit 4 R joystick JB2
 *              bit 3 R joystick JB1
 *              bit 2 R joystick JA2
 *              bit 1 R joystick JA1
 *              bit 0 R NE556 OUT (32Hz IC BJ)
 *                    W gate0 of PIT8253 (sound enable)
 *
 *  MZ800 memory map
 *
 *  0000-0FFF   ROM or RAM
 *  1000-1FFF   PCG ROM or RAM
 *  2000-7FFF   RAM
 *  8000-9FFF   videoram or RAM
 *  A000-BFFF   videoram or RAM
 *  C000-CFFF   PCG RAM or RAM
 *  D000-DFFF   videoram or RAM
 *  E000-FFFF   memory mapped IO or RAM
 *
 *  ToDo:
    - slows down while making sound
    - MZ800:
      - Port CF not done.
      - Dips not connected.
      - MZ800-mode display not working /Hi-res not coded.
      - The CRTC is a very complex custom device, mostly unemulated.
    - MZ1500:
      - Various ports not done.
      - Floppy disk and quick disk not done.
      - F4 display is blank.
      - Need manuals.

  Note: MZ800 hardware starts in memory map (mode A), but switches to MZ700
  compatibility mode (mode B) as soon as it starts up. We start in Mode B
  because it helps MZ1500 get started and it doesn't break anything.

*
 *****************************************************************************/

#include "emu.h"
#include "mz700.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/mz_cas.h"


/***************************************************************************
    TIMER DEVICE CALLBACKS
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mz_state::ne556_cursor_callback)
{
	m_cursor_bit ^= 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(mz_state::ne556_other_callback)
{
	m_other_timer ^= 1;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mz_state::mz700_mem(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0xcfff).ram();
	map(0xd000, 0xdfff).bankrw("bankd");
	map(0xe000, 0xffff).m(m_banke, FUNC(address_map_bank_device::amap8));
}

void mz_state::mz700_banke(address_map &map)
{
	// bank 0: ram (mz700_bank1)
	map(0x0000, 0x1fff).ram();
	// bank 1: devices (mz700_bank3)
	map(0x2000, 0x2003).mirror(0x1ff0).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x2004, 0x2007).mirror(0x1ff0).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2008, 0x200b).mirror(0x1ff0).rw(FUNC(mz_state::mz700_e008_r), FUNC(mz_state::mz700_e008_w));
	map(0x200c, 0x200f).mirror(0x1ff0).noprw();
	// bank 2: switched out (mz700_bank5)
	map(0x4000, 0x5fff).noprw();
}

void mz_state::mz700_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe0).w(FUNC(mz_state::mz700_bank_0_w));
	map(0xe1, 0xe1).w(FUNC(mz_state::mz700_bank_1_w));
	map(0xe2, 0xe2).w(FUNC(mz_state::mz700_bank_2_w));
	map(0xe3, 0xe3).w(FUNC(mz_state::mz700_bank_3_w));
	map(0xe4, 0xe4).w(FUNC(mz_state::mz700_bank_4_w));
	map(0xe5, 0xe5).w(FUNC(mz_state::mz700_bank_5_w));
	map(0xe6, 0xe6).w(FUNC(mz_state::mz700_bank_6_w));
}

void mz800_state::mz800_mem(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0x1fff).bankrw("bank1");
	map(0x2000, 0x7fff).ram();
	map(0x8000, 0xbfff).bankrw("banka");
	map(0xc000, 0xcfff).bankrw("bankc");
	map(0xd000, 0xdfff).bankrw("bankd");
	map(0xe000, 0xffff).m(m_bankf, FUNC(address_map_bank_device::amap8));
}

void mz800_state::mz800_bankf(address_map &map)
{
	// bank 0: ram (mz700_bank1)
	map(0x0000, 0x1fff).ram();
	// bank 1: devices (mz700_bank3)
	map(0x2000, 0x2003).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x2004, 0x2007).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2008, 0x200b).rw(FUNC(mz800_state::mz700_e008_r), FUNC(mz800_state::mz700_e008_w));
	map(0x200c, 0x200f).noprw();
	map(0x2010, 0x3fff).rom().region("monitor", 0x2010);
	// bank 2: switched out (mz700_bank5)
	map(0x4000, 0x5fff).noprw();
}

void mz800_state::mz800_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xcc, 0xcc).w(FUNC(mz800_state::mz800_write_format_w));
	map(0xcd, 0xcd).w(FUNC(mz800_state::mz800_read_format_w));
	map(0xce, 0xce).rw(FUNC(mz800_state::mz800_crtc_r), FUNC(mz800_state::mz800_display_mode_w));
	map(0xcf, 0xcf).w(FUNC(mz800_state::mz800_scroll_border_w));
	map(0xd0, 0xd3).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xd4, 0xd7).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xe0, 0xe0).rw(FUNC(mz800_state::mz800_bank_0_r), FUNC(mz800_state::mz800_bank_0_w));
	map(0xe1, 0xe1).rw(FUNC(mz800_state::mz800_bank_1_r), FUNC(mz800_state::mz700_bank_1_w));
	map(0xe2, 0xe2).w(FUNC(mz800_state::mz700_bank_2_w));
	map(0xe3, 0xe3).w(FUNC(mz800_state::mz700_bank_3_w));
	map(0xe4, 0xe4).w(FUNC(mz800_state::mz700_bank_4_w));
	map(0xe5, 0xe5).w(FUNC(mz800_state::mz700_bank_5_w));
	map(0xe6, 0xe6).w(FUNC(mz800_state::mz700_bank_6_w));
	map(0xea, 0xea).rw(FUNC(mz800_state::mz800_ramdisk_r), FUNC(mz800_state::mz800_ramdisk_w));
	map(0xeb, 0xeb).w(FUNC(mz800_state::mz800_ramaddr_w));
	map(0xf0, 0xf0).r(m_joy[0], FUNC(msx_general_purpose_port_device::read)).w(FUNC(mz800_state::mz800_palette_w));
	map(0xf1, 0xf1).r(m_joy[1], FUNC(msx_general_purpose_port_device::read));
	map(0xf2, 0xf2).w("sn76489n", FUNC(sn76489_device::write));
	map(0xfc, 0xff).rw("z80pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/* 2008-05 FP:
Notice that there is no Backspace key, only a 'Del' one.

Small note about natural keyboard support: currently,
- "Alpha" is mapped to 'F6'
- "Graph" is mapped to 'F7'
- "Break" is mapped to 'F8'                      */

static INPUT_PORTS_START( mz700 )
	PORT_START("ROW0")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, 0x08, IPT_UNUSED )
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_NAME("Alpha") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x93  \xC2\xA3") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(U'£') // this one would be 2nd row, 3rd key after 'P'
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)          PORT_CHAR('_') // this one would be 2nd row, 4th key after 'P'

	PORT_START("ROW1")
	PORT_BIT(0x07, 0x07, IPT_UNUSED )
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('Y') PORT_CHAR('y')

	PORT_START("ROW2")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('Q') PORT_CHAR('q')

	PORT_START("ROW3")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("ROW4")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('A') PORT_CHAR('a')

	PORT_START("ROW5")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW6")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_NAME("0  Pi")               PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x91  ~")     PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)          PORT_CHAR('\\') PORT_CHAR('|')  // this one would be 1st row, 3rd key after '0'

	PORT_START("ROW7")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("/  \xE2\x86\x90") PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("?  \xE2\x86\x92") PORT_CODE(KEYCODE_END)      PORT_CHAR('?')  // this one would be 4th row, 4th key after 'M'
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)                                  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                                 PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("DEL  HOME") PORT_CODE(KEYCODE_DEL)            PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_NAME("INST  CLR") PORT_CODE(KEYCODE_INSERT)         PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("ROW8")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x3e, 0x3e, IPT_UNUSED)
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_HOME)               PORT_CHAR(UCHAR_MAMEKEY(F8))    // this one would be at Backspace position

	PORT_START("ROW9")
	PORT_BIT(0x07, 0x07, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)            PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)            PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)            PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("JOY")
	PORT_BIT(0x01, 0x00, IPT_UNUSED)
	PORT_BIT(0x02, 0x00, IPT_JOYSTICK_UP)       PORT_8WAY
	PORT_BIT(0x04, 0x00, IPT_JOYSTICK_DOWN)     PORT_8WAY
	PORT_BIT(0x08, 0x00, IPT_JOYSTICK_LEFT)     PORT_8WAY
	PORT_BIT(0x10, 0x00, IPT_JOYSTICK_RIGHT)    PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( mz800 )
	PORT_INCLUDE(mz700)

	PORT_MODIFY("JOY")
	PORT_BIT(0x1f, 0x00, IPT_UNUSED)

	PORT_START("system_settings")
	PORT_DIPNAME(0x01, 0x00, "Mode selection")
	PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(0x01, "MZ-700")
	PORT_DIPSETTING(0x00, "MZ-800")
	PORT_DIPNAME(0x06, 0x06, "Printer selection")
	PORT_DIPLOCATION("SW:3,2")
	PORT_DIPSETTING(0x06, "MZ printer")
	PORT_DIPSETTING(0x00, "Centronics printer")
	PORT_DIPNAME(0x08, 0x08, "Cassette polarity")
	PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(0x08, DEF_STR(Unknown))
	PORT_DIPSETTING(0x00, DEF_STR(Unknown))
INPUT_PORTS_END


/***************************************************************************
    GFX LAYOUT
***************************************************************************/

static const gfx_layout mz700_layout =
{
	8, 8,       /* 8 x 8 graphics */
	512,        /* 512 codes */
	1,      /* 1 bit per pixel */
	{ 0 },      /* no bitplanes */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8       /* code takes 8 times 8 bits */
};

static GFXDECODE_START( gfx_mz700 )
	GFXDECODE_ENTRY("cgrom", 0, mz700_layout, 0, 4)
GFXDECODE_END

static GFXDECODE_START( gfx_mz800 )
	GFXDECODE_ENTRY("monitor", 0x1000, mz700_layout, 0, 4)    // for mz800 viewer only
GFXDECODE_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void mz_state::mz700(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(17'734'470)/5);
	m_maincpu->set_addrmap(AS_PROGRAM, &mz_state::mz700_mem);
	m_maincpu->set_addrmap(AS_IO, &mz_state::mz700_io);

	ADDRESS_MAP_BANK(config, "banke").set_map(&mz_state::mz700_banke).set_options(ENDIANNESS_LITTLE, 8, 16, 0x2000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(17'734'470)/2, 568, 0, 40*8, 312, 0, 25*8);
	m_screen->set_screen_update(FUNC(mz_state::screen_update_mz700));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::RGB_3BIT);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_mz700);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* ne556 timers */
	TIMER(config, "cursor").configure_periodic(FUNC(mz_state::ne556_cursor_callback), attotime::from_hz(1.5));
	TIMER(config, "other").configure_periodic(FUNC(mz_state::ne556_other_callback), attotime::from_hz(34.5));

	/* devices */
	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(17'734'470)/20);
	m_pit->out_handler<0>().set(FUNC(mz_state::pit_out0_changed));
	m_pit->set_clk<1>(15611.0);
	m_pit->out_handler<1>().set(m_pit, FUNC(pit8253_device::write_clk2));
	m_pit->set_clk<2>(0);
	m_pit->out_handler<2>().set(FUNC(mz_state::pit_irq_2));

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(mz_state::pio_port_a_w));
	m_ppi->in_pb_callback().set(FUNC(mz_state::pio_port_b_r));
	m_ppi->in_pc_callback().set(FUNC(mz_state::pio_port_c_r));
	m_ppi->out_pc_callback().set(FUNC(mz_state::pio_port_c_w));

	TTL74145(config, m_ls145);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(mz700_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("mz_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("mz700_cass");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}


void mz800_state::mz800(machine_config &config)
{
	mz700(config);
	config.device_remove("banke");

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &mz800_state::mz800_mem);
	m_maincpu->set_addrmap(AS_IO, &mz800_state::mz800_io);

	ADDRESS_MAP_BANK(config, "bankf").set_map(&mz800_state::mz800_bankf).set_options(ENDIANNESS_LITTLE, 8, 16, 0x2000);

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_mz800);

	m_screen->set_screen_update(FUNC(mz800_state::screen_update_mz800));

	SN76489(config, "sn76489n", XTAL(17'734'470)/5).add_route(ALL_OUTPUTS, "mono", 1.0);

	config.device_remove("cass_list");
	SOFTWARE_LIST(config, "cass_list").set_original("mz800_cass");

	/* devices */
	m_pit->set_clk<0>(XTAL(17'734'470)/16);

	m_ppi->out_pa_callback().set(FUNC(mz800_state::pio_port_a_w));

	z80pio_device& pio(Z80PIO(config, "z80pio", XTAL(17'734'470)/5));
	pio.out_int_callback().set_inputline(m_maincpu, 0);
	pio.in_pa_callback().set(FUNC(mz800_state::mz800_z80pio_port_a_r));
	pio.out_pa_callback().set(FUNC(mz800_state::mz800_z80pio_port_a_w));
	pio.out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));

	MSX_GENERAL_PURPOSE_PORT(config, m_joy[0], msx_general_purpose_port_devices, "joystick");
	MSX_GENERAL_PURPOSE_PORT(config, m_joy[1], msx_general_purpose_port_devices, "joystick");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( mz700 )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_LOAD( "1z-013a.rom", 0x0000, 0x1000, CRC(4c6c6b7b) SHA1(ef8f7399e86c1dc638a5cb83efdb73369c2b5735) )

	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "mz700fon.int", 0x0000, 0x1000, CRC(42b9e8fb) SHA1(5128ad179a702f8e0bd9910a58bad8fbe4c20167) )
ROM_END

ROM_START( mz700j )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_LOAD( "1z-009b.rom", 0x0000, 0x1000, CRC(ab1fbe6f) SHA1(7b10d7965c541393e33a265bcf71a00314d2db7a))

	ROM_REGION( 0x1000, "cgrom", 0 )
	//ROM_LOAD( "mz700fon.jp", 0x0000, 0x1000, CRC(697ec121) SHA1(5eb1d42d273b1fd2cab120486279ab8ff6c85dc7))
	ROM_LOAD( "mz700fon.jpn", 0x0000, 0x1000, CRC(425eedf5) SHA1(bd2cc750f2d2f63e50a59786668509e81a276e32) )
ROM_END

ROM_START( mz800 )
	ROM_REGION( 0x4000, "monitor", 0 )
	ROM_LOAD( "mz800.rom", 0x0000, 0x4000, CRC(600d17e1) SHA1(950ce4b51429916f8036e41ba6130fac149b36e4) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 ) // ramdisk
ROM_END

ROM_START( mz1500 )
	ROM_REGION( 0x4000, "monitor", 0 )
	ROM_LOAD( "9z-502m.rom",  0x0000, 0x1000, CRC(643db428) SHA1(c2ad8af2ef00db32afde54d5741b07de5d4da16a))
	ROM_CONTINUE(0x2800, 0x1800)

	ROM_REGION( 0x1000, "cgrom", 0 )
	//ROM_LOAD( "mz700fon.jp", 0x0000, 0x1000, CRC(697ec121) SHA1(5eb1d42d273b1fd2cab120486279ab8ff6c85dc7))
	ROM_LOAD( "mz700fon.jpn", 0x0000, 0x1000, CRC(425eedf5) SHA1(bd2cc750f2d2f63e50a59786668509e81a276e32) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 ) // ramdisk
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME          FLAGS
COMP( 1982, mz700,  0,      0,      mz700,   mz700, mz_state,    init_mz700, "Sharp", "MZ-700",         0 )
COMP( 1982, mz700j, mz700,  0,      mz700,   mz700, mz_state,    init_mz700, "Sharp", "MZ-700 (Japan)", 0 )
COMP( 1984, mz800,  0,      0,      mz800,   mz800, mz800_state, init_mz800, "Sharp", "MZ-800",         MACHINE_NOT_WORKING )
COMP( 1984, mz1500, 0,      0,      mz800,   mz800, mz800_state, init_mz800, "Sharp", "MZ-1500",        MACHINE_NOT_WORKING )    // Japanese version of the MZ-800
