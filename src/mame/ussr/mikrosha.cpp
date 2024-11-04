// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mikrosha driver by Miodrag Milanovic

        05/06/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "radio86.h"

#include "cpu/i8085/i8085.h"
#include "machine/pit8253.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/rk_cas.h"


namespace {

class mikrosha_state : public radio86_state
{
public:
	mikrosha_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag)
	{ }

	void mikrosha(machine_config &config);

private:
	void mikrosha_8255_font_page_w(uint8_t data);
	void mikrosha_pit_out2(int state);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_mikrosha_font_page = 0;
};

void mikrosha_state::machine_reset()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0x8000 + m_cart->get_rom_size() - 1, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));
	radio86_state::machine_reset();
	m_mikrosha_font_page = 0;
}

/* Address maps */
void mikrosha_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram"); // RAM
	map(0xc000, 0xc003).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write)).mirror(0x07fc);
	map(0xc800, 0xc803).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write)).mirror(0x07fc);
	map(0xd000, 0xd001).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write)).mirror(0x07fe); // video
	map(0xd800, 0xd803).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).mirror(0x07fc); // Timer
	map(0xe000, 0xf7ff).r(FUNC(mikrosha_state::radio_cpu_state_r)); // Not connected
	map(0xf800, 0xffff).rom().region("maincpu",0).w(m_dma, FUNC(i8257_device::write));    // DMA
}

void mikrosha_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0xff).rw(FUNC(mikrosha_state::radio_io_r), FUNC(mikrosha_state::radio_io_w));
}

/* Input ports */
static INPUT_PORTS_START( mikrosha )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PgUp") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0xA4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\\')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Center") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('~')

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PgDn") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

void mikrosha_state::mikrosha_8255_font_page_w(uint8_t data)
{
	m_mikrosha_font_page = (data >> 7) & 1;
}

void mikrosha_state::mikrosha_pit_out2(int state)
{
}

void mikrosha_state::machine_start()
{
	save_item(NAME(m_tape_value));
	save_item(NAME(m_mikrosha_font_page));
}

I8275_DRAW_CHARACTER_MEMBER(mikrosha_state::display_pixels)
{
	using namespace i8275_attributes;

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const *const charmap = &m_chargen[(m_mikrosha_font_page & 1) * 0x400];
	uint8_t pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (BIT(attrcode, VSP))
		pixels = 0;

	if (BIT(attrcode, LTEN))
		pixels = 0xff;

	if (BIT(attrcode, RVV))
		pixels ^= 0xff;

	bool hlgt = BIT(attrcode, HLGT);
	for(int i=0;i<6;i++)
		bitmap.pix(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
}

/* F4 Character Displayer */
static const gfx_layout mikrosha_charlayout =
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

static GFXDECODE_START( gfx_mikrosha )
	GFXDECODE_ENTRY( "chargen", 0x0000, mikrosha_charlayout, 0, 1 )
GFXDECODE_END

void mikrosha_state::mikrosha(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(16'000'000) / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &mikrosha_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mikrosha_state::io_map);

	I8255(config, m_ppi1);
	m_ppi1->in_pa_callback().set(FUNC(mikrosha_state::radio86_8255_portb_r2));
	m_ppi1->out_pb_callback().set(FUNC(mikrosha_state::radio86_8255_porta_w2));
	m_ppi1->in_pc_callback().set(FUNC(mikrosha_state::radio86_8255_portc_r2));
	m_ppi1->out_pc_callback().set(FUNC(mikrosha_state::radio86_8255_portc_w2));

	I8255(config, m_ppi2);
	m_ppi2->out_pb_callback().set(FUNC(mikrosha_state::mikrosha_8255_font_page_w));
	m_ppi2->tri_pb_callback().set_constant(0);

	i8275_device &i8275(I8275(config, "crtc", XTAL(16'000'000) / 12));
	i8275.set_character_width(6);
	i8275.set_display_callback(FUNC(mikrosha_state::display_pixels));
	i8275.drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(0);
	pit.set_clk<1>(0);
	pit.set_clk<2>(2000000);
	pit.out_handler<2>().set(FUNC(mikrosha_state::mikrosha_pit_out2));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(50);
	screen.set_size(78*6, 30*10);
	screen.set_visarea(0, 78*6-1, 0, 30*10-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mikrosha);
	PALETTE(config, m_palette, FUNC(mikrosha_state::radio86_palette), 3);

	SPEAKER(config, "mono").front_center();

	I8257(config, m_dma, XTAL(16'000'000) / 9);
	m_dma->out_hrq_cb().set(FUNC(mikrosha_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(mikrosha_state::memory_read_byte));
	m_dma->out_memw_cb().set(FUNC(mikrosha_state::memory_write_byte));
	m_dma->out_iow_cb<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma->set_reverse_rw_mode(1);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(rkm_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("mikrosha_cass");

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mikrosha_cart", "bin,rom");

	SOFTWARE_LIST(config, "cass_list").set_original("mikrosha_cass");
	SOFTWARE_LIST(config, "cart_list").set_original("mikrosha_cart");
}


/* ROM definition */
ROM_START( mikrosha )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mikrosha.rom", 0x0000, 0x0800, CRC(86a83556) SHA1(94b1baad0a419145939a891ff51f4324e8e4ddd2))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("mikrosha.fnt", 0x0000, 0x0800, CRC(b315da1c) SHA1(b5bf9abc0fff75b1aba709a7f08b23d4a89bb04b))
ROM_END

ROM_START( m86rk )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m86rk.bin", 0x0000, 0x0800, CRC(a898d77a) SHA1(c2497bf8434b5028fe0a9fc09be311465d5553a5))

	ROM_REGION(0x0800, "chargen",0)
	/* here should probably be different rom */
	ROM_LOAD ("mikrosha.fnt", 0x0000, 0x0800, CRC(b315da1c) SHA1(b5bf9abc0fff75b1aba709a7f08b23d4a89bb04b))
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT          COMPANY                                FULLNAME         FLAGS
COMP( 1987, mikrosha, radio86, 0,      mikrosha, mikrosha, mikrosha_state, init_radio86, "Lianozovo Electromechanical Factory", "Mikrosha",      MACHINE_SUPPORTS_SAVE )
COMP( 1987, m86rk,    radio86, 0,      mikrosha, mikrosha, mikrosha_state, init_radio86, "<unknown>",                           "Mikrosha-86RK", MACHINE_SUPPORTS_SAVE )
