// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Lola 8A

Ivo Lola Ribar Institute

2013-08-28 Skeleton driver.


BASIC commands : (must be in UPPERcase)

    LET NEXT IF GOTO GOSUB RETURN READ DATA FOR CLS INPUT DIM STOP END RESTORE
    REM CLEAR PUSH POKE PRINT OUT ERROR USR CURSOR NORMAL INVERSE PLOT UNPLOT
    ELSE WIPE COLOUR CENTRE RANGE DRAW CIRCLE LOAD SAVE VERIFY HLOAD HSAVE HVERIFY
    DLOAD DSAVE DVERIFY MERGE CAT RUN NEW ON LIST DEF MON GWIND TWIND UNDER
    SPC OFF TAB THEN TO STEP AND OR XOR NOT ABS LEN SQR INT ASC CHR VAL STR MID
    ARG CALL RND LEFT RIGHT DOT SGN SIN FREE PI FN TAN COS POP PEEK INP LN EXP ATN

COLOUR x (x = 0 to 3) there's no known colour ram, unable to determine
how colours can be displayed. Therefore we only show black and white.

Unknown how to produce sound - there's no commands.

MON: (Guesswork by trying things)
    - A0 : display addresses A0 to A7
    - 0,FFF : display addresses 0 to FFE (yes it leaves one out)
    - G,R : displays registers (R B=3 : set B register to 3)
    - M,N,Q,T : display or set a specific "register"? (Q : display Q ; Q=6 : set Q to 6)
    - SP : display or set SP (SP=3B00 : set SP)
    - PC : display or set PC
    - L : disassembler (L 0,FF : disassemble range 0 to FF)
    - K : single-step? (address set by G) (K 99 : single-step at 99)

Control Keys
    - A : display gets a little corrupted
    - G : back to normal
    - C : break? (only does a newline)
    - L : clear screen
    - M : same as pressing Enter

TO DO
    - How to use the sound?
    - Does it have colour?
    - What is Ctrl-A for?
    - Need software
    - Need manuals & schematics

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define AY8910_TAG "g12"
#define HD46505SP_TAG "h45"


namespace {

class lola8a_state : public driver_device
{
public:
	lola8a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_cass(*this, "cassette")
		, m_palette(*this, "palette")
		, m_p_videoram(*this, "videoram")
		, m_io_keyboard(*this, "KEY.%u", 0U)
		, m_io_modifier(*this, "MODIFIER")
	{ }

	void lola8a(machine_config &config);

private:
	void machine_reset() override;
	void machine_start() override;
	u8 lola8a_port_a_r();
	void lola8a_port_b_w(u8 data);
	void crtc_vsync(int state);
	int cass_r();
	void cass_w(int state);
	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u8 m_portb = 0U;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<i8085a_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cass;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_p_videoram;
	required_ioport_array<10> m_io_keyboard;
	required_ioport m_io_modifier;
};

void lola8a_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// Sockets for RAM 6264 at G45, F45, E45 and D45
	// should be populated in order, usual configuration
	// is 16K with G45 and F45 populated
	map(0x8000, 0x9fff).rom().region("maincpu", 0); // 2764A at B45
	map(0xa000, 0xbfff).rom().region("maincpu", 0x2000); // 2764A at C45
	map(0xc000, 0xdfff).rom().region("maincpu", 0x4000); // 2764A at H67
	map(0xe000, 0xffff).ram().share("videoram"); // 6264 at G67
}

void lola8a_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x80, 0x80).w(AY8910_TAG, FUNC(ay8910_device::address_w));
	map(0x84, 0x84).w(AY8910_TAG, FUNC(ay8910_device::data_w));
	map(0x88, 0x88).r(AY8910_TAG, FUNC(ay8910_device::data_r));
	map(0x90, 0x90).rw(HD46505SP_TAG, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x92, 0x92).rw(HD46505SP_TAG, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

/* Input ports */
static INPUT_PORTS_START( lola8a )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(U'Ž') PORT_CHAR(U'ž')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Č') PORT_CHAR(U'č')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(U'Š') PORT_CHAR(U'š')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'Ć') PORT_CHAR(U'ć')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(':') PORT_CHAR('@')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RET") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIER")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
INPUT_PORTS_END

MC6845_UPDATE_ROW( lola8a_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	ma &= 0x7ff;

	for (u8 x = 0; x < x_count; x++)
	{
		u16 mem = (x+ma)*8 + ra;
		u8 gfx = m_p_videoram[mem] ^ ((cursor_x == x) ? 0xff : 0);

		*p++ = palette[BIT(gfx, 7) ? 7 : 0];
		*p++ = palette[BIT(gfx, 6) ? 7 : 0];
		*p++ = palette[BIT(gfx, 5) ? 7 : 0];
		*p++ = palette[BIT(gfx, 4) ? 7 : 0];
		*p++ = palette[BIT(gfx, 3) ? 7 : 0];
		*p++ = palette[BIT(gfx, 2) ? 7 : 0];
		*p++ = palette[BIT(gfx, 1) ? 7 : 0];
		*p++ = palette[BIT(gfx, 0) ? 7 : 0];
	}
}


u8 lola8a_state::lola8a_port_a_r()
{
	u8 data = 0xff, kbrow = m_portb & 15;

	if (kbrow < 10)
		data = m_io_keyboard[kbrow]->read();

	return data & m_io_modifier->read();
}

void lola8a_state::lola8a_port_b_w(u8 data)
{
	m_portb = data;
}

int lola8a_state::cass_r()
{
	return (m_cass->input() < 0.03);
}

void lola8a_state::cass_w(int state)
{
	m_cass->output(state ? -1.0 : +1.0);
}


void lola8a_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x1fff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x8000, 0x9fff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x1fff, m_ram->pointer());
				}
			},
			&m_rom_shadow_tap);
}

void lola8a_state::machine_start()
{
	// Add just RAM over 8K, first 8K will be added after reset
	if (m_ram->size() > 8 * 1024)
		m_maincpu->space(AS_PROGRAM).install_ram(0x2000, m_ram->size() - 1, m_ram->pointer() + 0x2000);
	save_item(NAME(m_portb));
}

void lola8a_state::crtc_vsync(int state)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state? ASSERT_LINE : CLEAR_LINE);
}

void lola8a_state::lola8a(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &lola8a_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lola8a_state::io_map);
	m_maincpu->in_sid_func().set(FUNC(lola8a_state::cass_r));
	m_maincpu->out_sod_func().set(FUNC(lola8a_state::cass_w));

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, AY8910_TAG, XTAL(4'915'200) / 4));
	aysnd.port_a_read_callback().set(FUNC(lola8a_state::lola8a_port_a_r));
	aysnd.port_b_write_callback().set(FUNC(lola8a_state::lola8a_port_b_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8_MHz_XTAL, 512, 0, 320, 313, 0, 200);
	screen.set_screen_update(HD46505SP_TAG, FUNC(hd6845s_device::screen_update));

	hd6845s_device &crtc(HD6845S(config, HD46505SP_TAG, XTAL(8'000'000) / 8)); // HD6845 == HD46505S
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(lola8a_state::crtc_update_row));
	crtc.out_vsync_callback().set(FUNC(lola8a_state::crtc_vsync));

	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	/* Cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("8K,16K,24K,32K");
}

/* ROM definition */
ROM_START( lola8a )
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lola 8a r0 w06 22.11.86.b45", 0x0000, 0x2000, CRC(aca1fc08) SHA1(f7076d937bb53b0addcba2a5b7c05ab75d6d0d93))
	ROM_LOAD( "lola 8a r1 w06 22.11.86.c45", 0x2000, 0x2000, CRC(99f8ec9b) SHA1(88eafd09c479f177525fa0039cf04d74bae39dab))
	ROM_LOAD( "lola 8a r2 w06 22.11.86.h67", 0x4000, 0x2000, CRC(1e7cd46b) SHA1(048b2583ee7baeb9621e629b79ed64583ac5d554))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                    FULLNAME   FLAGS
COMP( 1986, lola8a, 0,      0,      lola8a,  lola8a, lola8a_state, empty_init, "Institut Ivo Lola Ribar", "Lola 8A", MACHINE_SUPPORTS_SAVE )
