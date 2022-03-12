// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Apogee BK-01 driver by Miodrag Milanovic

        05/06/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/radio86.h"

#include "cpu/i8085/i8085.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/rk_cas.h"


namespace {

class apogee_state : public radio86_state
{
public:
	apogee_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag)
		, m_speaker(*this, "speaker")
	{ }

	void apogee(machine_config &config);

private:
	uint8_t m_out0;
	uint8_t m_out1;
	uint8_t m_out2;
	DECLARE_WRITE_LINE_MEMBER(pit8253_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(pit8253_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(pit8253_out2_changed);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);

	required_device<speaker_sound_device> m_speaker;
	void mem_map(address_map &map);
	void machine_reset() override;
	void machine_start() override;
};


/* Address maps */
void apogee_state::mem_map(address_map &map)
{
	map(0x0000, 0xebff).ram().share("mainram");
	map(0xec00, 0xec03).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).mirror(0x00fc);
	map(0xed00, 0xed03).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write)).mirror(0x00fc);
	//map(0xee00, 0xee03).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write)).mirror(0x00fc);
	map(0xef00, 0xef01).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write)).mirror(0x00fe); // video
	map(0xf000, 0xf0ff).w(m_dma, FUNC(i8257_device::write));    // DMA
	map(0xf000, 0xffff).rom().region("maincpu",0);  // System ROM
}

/* Input ports */
static INPUT_PORTS_START( apogee )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PgUp") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PgDn") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0xA4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
INPUT_PORTS_END

static const double speaker_levels[] = {-1.0, -0.33333, 0.33333, 1.0};

WRITE_LINE_MEMBER(apogee_state::pit8253_out0_changed)
{
	m_out0 = state;
	m_speaker->level_w(m_out0+m_out1+m_out2);
}

WRITE_LINE_MEMBER(apogee_state::pit8253_out1_changed)
{
	m_out1 = state;
	m_speaker->level_w(m_out0+m_out1+m_out2);
}

WRITE_LINE_MEMBER(apogee_state::pit8253_out2_changed)
{
	m_out2 = state;
	m_speaker->level_w(m_out0+m_out1+m_out2);
}

void apogee_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x0fff, m_rom+0x0800);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf000, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0fff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void apogee_state::machine_start()
{
	save_item(NAME(m_tape_value));
	save_item(NAME(m_out0));
	save_item(NAME(m_out1));
	save_item(NAME(m_out2));
}

I8275_DRAW_CHARACTER_MEMBER(apogee_state::display_pixels)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const *const charmap = &m_chargen[(gpa & 1) * 0x400];
	uint8_t pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp)
		pixels = 0;

	if (lten)
		pixels = 0xff;

	if (rvv)
		pixels ^= 0xff;

	for(int i=0;i<6;i++)
		bitmap.pix(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
}

/* F4 Character Displayer */
static const gfx_layout apogee_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_apogee )
	GFXDECODE_ENTRY( "chargen", 0x0000, apogee_charlayout, 0, 1 )
GFXDECODE_END


/* Machine driver */
void apogee_state::apogee(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(16'000'000) / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &apogee_state::mem_map);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(XTAL(16'000'000)/9);
	pit.out_handler<0>().set(FUNC(apogee_state::pit8253_out0_changed));
	pit.set_clk<1>(XTAL(16'000'000)/9);
	pit.out_handler<1>().set(FUNC(apogee_state::pit8253_out1_changed));
	pit.set_clk<2>(XTAL(16'000'000)/9);
	pit.out_handler<2>().set(FUNC(apogee_state::pit8253_out2_changed));

	I8255(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(apogee_state::radio86_8255_porta_w2));
	m_ppi1->in_pb_callback().set(FUNC(apogee_state::radio86_8255_portb_r2));
	m_ppi1->in_pc_callback().set(FUNC(apogee_state::radio86_8255_portc_r2));
	m_ppi1->out_pc_callback().set(FUNC(apogee_state::radio86_8255_portc_w2));

	//I8255(config, m_ppi2);

	i8275_device &i8275(I8275(config, "crtc", XTAL(16'000'000) / 12));
	i8275.set_character_width(6);
	i8275.set_display_callback(FUNC(apogee_state::display_pixels));
	i8275.drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(50);
	screen.set_size(78*6, 30*10);
	screen.set_visarea(0, 78*6-1, 0, 30*10-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_apogee);
	PALETTE(config, m_palette, FUNC(apogee_state::radio86_palette), 3);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.75);

	I8257(config, m_dma, XTAL(16'000'000) / 9);
	m_dma->out_hrq_cb().set(FUNC(apogee_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(apogee_state::memory_read_byte));
	m_dma->out_memw_cb().set(FUNC(apogee_state::memory_write_byte));
	m_dma->out_iow_cb<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma->set_reverse_rw_mode(1);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_formats(rka_cassette_formats);
	m_cassette->set_interface("apogee_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("apogee");
}

/* ROM definition */
ROM_START( apogee )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "apogee.rom", 0x0000, 0x1000, CRC(a47383a7) SHA1(6a868371c7980f92c2fc9ced921517209f197375))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("apogee.fnt", 0x0000, 0x0800, CRC(fe5867f0) SHA1(82c5aca63ada5e4533eb0516384aaa7b77a1f8e2))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT          COMPANY      FULLNAME        FLAGS
COMP( 1989, apogee, radio86, 0,      apogee,  apogee, apogee_state, init_radio86, "Zavod BRA", "Apogee BK-01", MACHINE_SUPPORTS_SAVE )
