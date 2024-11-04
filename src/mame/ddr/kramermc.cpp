// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Kramer MC driver by Miodrag Milanovic

2008-09-13 Preliminary driver.

This is a homebrew computer by Manfred Kramer, of Germany. The "Y23VO" that
appears at startup was the author's amateur radio callsign at the time. (It
changed after reunification.)

The computer was intended to have a CTC, a SIO, and a FDC (uPD765A), however
there's no code to make use of them. The system PIO is intended for
keyboard, serial, and beeper.

Commands:
A : Assembler
B : Basic
C : Copy
D : Dump (Hex dump of memory)
E : End-file
F : Fill
G : Go
H : Hex Arithmetic
I : Input device (IT or IL or IU)
J : Jump ram
K : Ram test
L : List device (LT or LL or LU)
M : Move
O : Output device (OT or OL or OU)
P : Disassemble
R : Read external storage (expects intel format from the keyboard)
S : Substitute (edit ram content)
T : Text editor
V : Verify
W : Write (sends bits to the PIO port A)
X : Show primary registers
Z : Show secondary registers

Device choices: T (terminal), L (cassette), U (user device)

There's numerous bugs in the rom's keyboard routine. Shift has to be pressed
and released before the key you want to shift. Lowercase cannot be input. The
Ctrl key doesn't work. The . , / keys are incorrectly already shifted.

Even though cassette and terminal support are supposed to exist, the device
redirection commands do nothing.

The natural keyboard has been added, although shifted characters will not
work, or for paste either, because of the shift-key bug. Further, paste
drops every second character.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "screen.h"
#include "emupal.h"
#include "sound/spkrdev.h"
#include "speaker.h"


namespace {

class kramermc_state : public driver_device
{
public:
	kramermc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_videoram(*this, "videoram")
		, m_speaker(*this, "speaker")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "LINE%u", 0)
	{ }

	void kramermc(machine_config &config);

	void init_kramermc();

private:
	u8 m_porta = 0U;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_kramermc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t port_a_r();
	uint8_t port_b_r();
	void port_a_w(uint8_t data);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u8> m_videoram;
	required_device<speaker_sound_device> m_speaker;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_io_keyboard;
	void kramermc_io(address_map &map) ATTR_COLD;
	void kramermc_mem(address_map &map) ATTR_COLD;
};


/* Address maps */
void kramermc_state::kramermc_mem(address_map &map)
{
	map(0x0000, 0x03ff).rom();  // Monitor
	map(0x0400, 0x07ff).rom();  // Debugger
	map(0x0800, 0x0bff).rom();  // Reassembler
	map(0x0c00, 0x0fff).ram();  // System RAM
	map(0x1000, 0x7fff).ram();  // User RAM
	map(0x8000, 0xafff).rom();  // BASIC
	map(0xc000, 0xc3ff).rom();  // Editor
	map(0xc400, 0xdfff).rom();  // Assembler
	map(0xfc00, 0xffff).ram().share("videoram");  // Video RAM
}

void kramermc_state::kramermc_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xfc, 0x0ff).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

/* Input ports */
static INPUT_PORTS_START( kramermc )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_BACKSLASH)     // It cancels the input line
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') PORT_CHAR(',')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)   // If pressed twice, prints a graphic minus
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)  // Prints underscore
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(34)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)  // Prints a graphic cube
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CODE(KEYCODE_STOP) PORT_CHAR('>') PORT_CHAR('.')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_START("LINE7")
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

uint8_t kramermc_state::port_a_r()
{
	return m_porta;
}

uint8_t kramermc_state::port_b_r()
{
	u8 key_row = (m_porta >> 1) & 7;
	return m_io_keyboard[key_row]->read();
}

void kramermc_state::port_a_w(uint8_t data)
{
	m_porta = data;
	m_speaker->level_w(BIT(data, 5));
}

void kramermc_state::machine_start()
{
	save_item(NAME(m_porta));
}

void kramermc_state::machine_reset()
{
	m_porta = 0xff;
}

const gfx_layout kramermc_charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0, 1, 2, 3, 4, 5, 6, 7},
	{0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                 /* size of one char */
};

static GFXDECODE_START( gfx_kramermc )
	GFXDECODE_ENTRY( "chargen", 0x0000, kramermc_charlayout, 0, 1 )
GFXDECODE_END

uint32_t kramermc_state::screen_update_kramermc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(u8 y = 0; y < 16; y++ )
		for(u8 x = 0; x < 64; x++ )
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_videoram[y*64+x], 0, 0, 0, x*8, y*8);

	return 0;
}

/* Machine driver */
void kramermc_state::kramermc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 1500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &kramermc_state::kramermc_mem);
	m_maincpu->set_addrmap(AS_IO, &kramermc_state::kramermc_io);

	z80pio_device& pio(Z80PIO(config, "pio", 1500000));
	pio.in_pa_callback().set(FUNC(kramermc_state::port_a_r));
	pio.out_pa_callback().set(FUNC(kramermc_state::port_a_w));
	pio.in_pb_callback().set(FUNC(kramermc_state::port_b_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 16*8);
	screen.set_visarea(0, 64*8-1, 0, 16*8-1);
	screen.set_screen_update(FUNC(kramermc_state::screen_update_kramermc));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kramermc);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);
}

/* ROM definition */
ROM_START( kramermc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "io-mon.kmc",   0x0000, 0x0400, CRC(ba230fc8) SHA1(197d4ede31ee8768dd4a17854ee21c468e98b3d6) )
	ROM_LOAD( "debugger.kmc", 0x0400, 0x0400, CRC(5ea3d9e1) SHA1(42e5ced4f965124ae50ec7ac9861d6b668cfab99) )
	ROM_LOAD( "reass.kmc",    0x0800, 0x0400, CRC(7cc8e605) SHA1(3319a96aad710441af30dace906b9725e07ca92c) )
	ROM_LOAD( "basic.kmc",    0x8000, 0x3000, CRC(7531801e) SHA1(61d055495ffcc4a281ef0abc3e299ea95f42544b) )
	ROM_LOAD( "editor.kmc",   0xc000, 0x0400, CRC(2fd4cb84) SHA1(505615a218865aa8becde13848a23e1241a14b96) )
	ROM_LOAD( "ass.kmc",      0xc400, 0x1c00, CRC(9a09422e) SHA1(a578d2cf0ea6eb35dcd13e4107e15187de906097) )

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("chargen.kmc",  0x0000, 0x0800, CRC(1ba52f9f) SHA1(71bbad90dd427d0132c871a4d3848ab3d4d84b8a) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY           FULLNAME       FLAGS */
COMP( 1987, kramermc, 0,      0,      kramermc, kramermc, kramermc_state, empty_init, "Manfred Kramer", "Kramer MC", MACHINE_SUPPORTS_SAVE )

