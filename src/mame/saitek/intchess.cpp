// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Achim
/*******************************************************************************

Intelligent Games / SciSys Intelligent Chess

Developed by Intelligent Games, the same group of people that worked on SciSys
Super System III and Mark V. Manufactured by SciSys, their main business partner
at the time. The visual interface is an evolution of "Tolinka".

It was advertised in a 1980 brochure by SciSys, but it looks like SciSys didn't
sell this chess computer. It was marketed by Intelligent Games themselves.
The UK version wasn't widely released, the German version was more common.

Hardware notes:
- PCB label: INTELLIGENT GAMES Ltd, (C) 1980, IG3
- Synertek 6502A @ ~1.1MHz
- Synertek 6522 VIA
- 2*4KB ROM(Synertek 2332), 2KB RAM(4*M5L2114LP)
- 256 bytes PROM(MMI 6336-1J), 256x4 VRAM(2101-1), RF video
- MM74C923N keyboard encoder, 20 buttons
- cassette deck with microphone
- 4-digit 7seg display

TODO:
- colors are estimated from photos (black and white are obvious, but the green
  and cyan are not standard 0x00ff00 / 0x00ffff)
- video timing is unknown, sprite offsets are estimated from photos

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6522via.h"
#include "machine/mm74c922.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "intchess.lh"


namespace {

class intchess_state : public driver_device
{
public:
	intchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via"),
		m_encoder(*this, "encoder"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_vram(*this, "vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_cass(*this, "cassette")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void intchess(machine_config &config);

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<mm74c923_device> m_encoder;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_shared_ptr<u8> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<cassette_image_device> m_cass;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void seg_w(u8 data);
	void control_w(u8 data);
	u8 control_r();

	void init_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vram_w(offs_t offset, u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(cass_input);
};

INPUT_CHANGED_MEMBER(intchess_state::reset_button)
{
	// assume that reset button is tied to 6502/6522
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	if (newval)
		m_via->reset();
}



/*******************************************************************************
    Video
*******************************************************************************/

void intchess_state::init_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xb0, 0xd0, 0xff); // cyan
	palette.set_pen_color(1, 0x00, 0x00, 0x00); // black
	palette.set_pen_color(2, 0x88, 0xa8, 0x50); // green
	palette.set_pen_color(3, 0xff, 0xff, 0xff); // white
}

u32 intchess_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw chessboard background
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
			bitmap.pix(y, x) = ((x / 20) ^ (y / 16)) << 1 & 2;

	// draw the sprites
	for (int i = 0; i < 64; i++)
	{
		int code = m_vram[i] & 7;
		int color = m_vram[i] >> 3 & 1;
		int x = (i % 8) * 20 + 2;
		int y = (i / 8) * 16;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, code, color, 0, 0, x, y, 0);
	}

	return 0;
}

void intchess_state::vram_w(offs_t offset, u8 data)
{
	// d0-d2: sprite index
	// d3: color
	// d4-d7: N/C (4-bit RAM chip)
	m_vram[offset] = data & 0xf;
}



/*******************************************************************************
    I/O
*******************************************************************************/

void intchess_state::seg_w(u8 data)
{
	// PA1-PA7: 7seg data
	// PA0: ?
	m_display->write_mx(bitswap<8>(~data,0,1,2,3,4,5,6,7));
}

void intchess_state::control_w(u8 data)
{
	// PB0-PB3: digit select
	m_display->write_my(data & 0xf);

	// PB5-PB7 to cassette deck
	// PB5: speaker
	m_dac->write(BIT(data, 5));

	// PB6: ?
	// PB7: cassette output
	m_cass->output(BIT(data, 7) ? +1.0 : -1.0);
}

u8 intchess_state::control_r()
{
	// PB4: 74C923 data available
	return m_encoder->da_r() ? 0x10 : 0x00;
}

TIMER_DEVICE_CALLBACK_MEMBER(intchess_state::cass_input)
{
	// cassette input is tied to NMI
	bool state = ((m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY) && (m_cass->input() < -0.04);
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void intchess_state::main_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0400).ram();
	map(0x0800, 0x0bff).mirror(0x0400).ram();
	map(0x1000, 0x1000).r(m_encoder, FUNC(mm74c923_device::read));
	map(0x1800, 0x18ff).ram().w(FUNC(intchess_state::vram_w)).share("vram");
	map(0xa800, 0xa80f).m(m_via, FUNC(via6522_device::map));
	map(0xc000, 0xdfff).mirror(0x2000).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( intchess ) // see comments for German version labels
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / Clear Square")                   // Spielstärke / Feld Frei
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")   // Löschen
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Flash")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2 / Knight")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6 / King")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Clear Board")                 // Neue Partie / Brett Frei
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") // Eingabe
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back") // Zurück

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3 / Bishop")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7 / White")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Mode")      // Modus
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Find")      // Check
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Next Best") // Altern

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8 / Black")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Record")    // Speichern
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Place")     // Setzen
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Step")      // Vor

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, intchess_state, reset_button, 0) PORT_NAME("Reset") // Start
INPUT_PORTS_END



/*******************************************************************************
    GFX Layouts
*******************************************************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(8*16,1), STEP8(0,1) },
	{ STEP16(0,1*8) },
	16*16
};

static GFXDECODE_START( gfx_intchess )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout, 0, 2 )
GFXDECODE_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void intchess_state::intchess(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 4.433619_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &intchess_state::main_map);

	MOS6522(config, m_via, 4.433619_MHz_XTAL / 4); // DDRA = 0xff, DDRB = 0xef
	m_via->writepa_handler().set(FUNC(intchess_state::seg_w));
	m_via->writepb_handler().set(FUNC(intchess_state::control_w));
	m_via->readpb_handler().set(FUNC(intchess_state::control_r));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MM74C923(config, m_encoder, 0); // timing parameters unknown
	m_encoder->da_wr_callback().set(m_via, FUNC(via6522_device::write_ca2));
	m_encoder->x1_rd_callback().set_ioport("X1");
	m_encoder->x2_rd_callback().set_ioport("X2");
	m_encoder->x3_rd_callback().set_ioport("X3");
	m_encoder->x4_rd_callback().set_ioport("X4");
	m_encoder->data_tri_callback().set(m_encoder, FUNC(mm74c923_device::read));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50); // PAL
	m_screen->set_size(8*20 + 32, 8*16 + 32);
	m_screen->set_visarea(0, 8*20-1, 0, 8*16-1);
	m_screen->set_screen_update(FUNC(intchess_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(m_via, FUNC(via6522_device::write_cb2));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_intchess);
	PALETTE(config, m_palette, FUNC(intchess_state::init_palette), 4);

	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_intchess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "speaker", 0.05);
	TIMER(config, "cass_input").configure_periodic(FUNC(intchess_state::cass_input), attotime::from_usec(10));
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( intchess )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c45015_ytv-lrom.u9", 0xc000, 0x1000, CRC(eef04467) SHA1(5bdcb8d596b91aa06c6ef1ed53ef14d0d13f4194) ) // 2332
	ROM_LOAD("c45016_ytv-hrom.u8", 0xd000, 0x1000, CRC(7e6f85b4) SHA1(4cd15257eae04067160026f9a062a28581f46227) ) // "

	ROM_REGION( 0x100, "sprites", 0 )
	ROM_LOAD("igp.u15", 0x000, 0x100, CRC(bf8358e0) SHA1(880e0d9bd8a75874ba9e51dfb5999b8fcd321a4f) ) // 6336-1
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, intchess, 0,      0,      intchess, intchess, intchess_state, empty_init, "Intelligent Games / SciSys", "Intelligent Chess", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS )
