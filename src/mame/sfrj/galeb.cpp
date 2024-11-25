// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Galeb driver by Miodrag Milanovic

2008-02-22 Preliminary driver.
2008-02-23 Sound support added.
2008-03-01 Updated to work with latest SVN code

Driver is based on work of Josip Perusanec.

Galeb (seagull) based on the OSI UK101. Due to its expense it is quite rare.
It was succeeded by the Orao (eagle).

All commands in Monitor and in Basic must be in UPPERCASE. So, the very first
thing to do is to press ^A to enter caps mode.

Commands:
A nnnn           Mini-assembler
B
C aaaabbbb       show checksum of memory from a to b
E aaaabbbb       Hex dump from a to b
F aaaabbbbcc     Fill memory from a to b with c
L                Load tape
M nnnn           Modify memory starting at byte nnnn (enter to escape)
P
Q ccccaaaabbbb   Copy memory range a to b, over to c
R
S
T
U nnnn           Go to nnnn
X nnnn           disassemble
$
^A               caps lock
^B               Start Basic
^G               keyclick
^L               clear screen
Alt\


Cassette:
- Unable to locate a schematic, but it appears that a 6850 is the uart.
  The UK101 used a 6850 and Kansas City tape encoding, and so it is assumed
  that this computer is the same.

Screen:
- The T command can cause the system to expand the video to 4k
  instead of 1k. Therefore the writing goes off the bottom and doesn't scroll.
  It is not known if this is intentional, or a bug. The video routine has been
  modified to cope, in case it's a real feature. (T 0000 0022 is an example)

ToDo:
- BASIC SAVE command is weird... need instructions
- Therefore, cassette is considered to be not working.

****************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/clock.h"
#include "screen.h"
#include "speaker.h"
#include "machine/6850acia.h"
#include "machine/timer.h"
#include "imagedev/cassette.h"
#include "sound/dac.h"
#include "emupal.h"

namespace {

class galeb_state : public driver_device
{
public:
	galeb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "mainram")
		, m_vram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_cass(*this, "cassette")
		, m_acia(*this, "acia")
		, m_dac(*this, "dac")
	{ }

	void galeb(machine_config &config);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void dac_w(u8 data);
	u8 keyboard_r(offs_t offset);
	u32 screen_update_galeb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u8 m_cass_data[4]{};
	bool m_cassbit = false;
	bool m_cassold = false;
	required_shared_ptr<u8> m_ram;
	required_shared_ptr<u8> m_vram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_io_keyboard;
	required_device<cassette_image_device> m_cass;
	required_device<acia6850_device> m_acia;
	required_device<dac_1bit_device> m_dac;

	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	bool m_dac_state = false;
};

void galeb_state::machine_start()
{
	save_item(NAME(m_dac_state));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void galeb_state::machine_reset()
{
	m_dac_state = 0;
	m_dac->write(m_dac_state);
}

void galeb_state::dac_w(uint8_t data)
{
	m_dac_state ^= 1;
	m_dac->write(m_dac_state);
}

u8 galeb_state::keyboard_r(offs_t offset)
{
	return m_io_keyboard[offset]->read();
}

TIMER_DEVICE_CALLBACK_MEMBER( galeb_state::kansas_w )
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER( galeb_state::kansas_r)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	u8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_acia->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}


/* Address maps */
void galeb_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("mainram");
	map(0xb000, 0xbfdf).ram().share("videoram");
	map(0xbfe0, 0xbfe7).r(FUNC(galeb_state::keyboard_r));
	map(0xbfe0, 0xbfe0).w(FUNC(galeb_state::dac_w));
	map(0xbffe, 0xbfff).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xc000, 0xffff).rom().region("maincpu",0);
}

/* Input ports */
static INPUT_PORTS_START( galeb )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GR") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down - Up") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'đ') PORT_CHAR(U'Đ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left - Right") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'š') PORT_CHAR(U'Š')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'ć') PORT_CHAR(U'Ć')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(U'ž') PORT_CHAR(U'Ž')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'č') PORT_CHAR(U'Č')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

const gfx_layout charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

static GFXDECODE_START( gfx_galeb )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

u32 galeb_state::screen_update_galeb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 ma = (m_ram[0xff]*256+m_ram[0xfe]) & 0xfff;
	if (ma < 0x400)
		ma = 15;
	else
		ma -= 0x3bb;

	for(u8 y = 0; y < 16; y++ )
	{
		for(u8 x = 0; x < 48; x++ )
		{
			u8 code = m_vram[ma + x + y*64];
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,  code , 0, 0, 0, x*8,y*8);
		}
	}
	return 0;
}

/* Machine driver */
void galeb_state::galeb(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &galeb_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(48*8, 16*8);
	screen.set_visarea(0, 48*8-1, 0, 16*8-1);
	screen.set_screen_update(FUNC(galeb_state::screen_update_galeb));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galeb);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.0625); // unknown DAC

	clock_device &acia_clock(CLOCK(config, "acia_clock", 4'800)); // 300 baud x 16(divider) = 4800
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(galeb_state::kansas_w), attotime::from_hz(4800)); // cass write
	TIMER(config, "kansas_r").configure_periodic(FUNC(galeb_state::kansas_r), attotime::from_hz(40000)); // cass read
}

/* ROM definition */
ROM_START( galeb )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	// BASIC ROMS
	ROM_LOAD( "bas01.rom",   0x0000, 0x0800, CRC(9b19ed58) SHA1(ebfc27af8dbabfb233f9888e6a0a0dfc87ae1691) )
	ROM_LOAD( "bas02.rom",   0x0800, 0x0800, CRC(3f320a84) SHA1(4ea082b4269dca6152426b1f720c7508122d3cb7) )
	ROM_LOAD( "bas03.rom",   0x1000, 0x0800, CRC(f122ad10) SHA1(3c7c1dd67268230d179a00b0f8b35be80c2b7035) )
	ROM_LOAD( "bas04.rom",   0x1800, 0x0800, CRC(b5372a83) SHA1(f93b73d98b943c6791f46617418fb5e4238d75bd) )
	// MONITOR
	ROM_LOAD( "exmd.rom",    0x3000, 0x0800, CRC(1bcb1375) SHA1(fda3361d238720a3d309644093da9832d5aff661) )
	// SYSTEM
	ROM_LOAD( "makbug.rom",  0x3800, 0x0800, CRC(91e38e79) SHA1(2b6439a09a470cda9c81b9d453c6380b99716989) )

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("chrgen.bin",  0x0000, 0x0800, CRC(409a800e) SHA1(0efe429dd6c0568032636e691d9865a623afeb55) )
ROM_END

} // Anonymous namespace

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME  FLAGS
COMP( 1981, galeb, 0,      0,      galeb,   galeb, galeb_state, empty_init, "PEL Varazdin", "Galeb",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
