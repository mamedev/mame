// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Atari Quiz Show
  released under their Kee Games label, 04/1976

  S2650 CPU, 512 bytes RAM, B&W tilemapped video. It uses a tape player to
  stream questions, totaling 1000, divided into 4 categories.

TODO:
- tape recordings are available, but there are DC offset problems
- MAME needs multi-track cassette support
- is timing accurate?

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "quizshow.lh"


namespace {

static constexpr XTAL MASTER_CLOCK  = 12.096_MHz_XTAL;
static constexpr XTAL PIXEL_CLOCK   = MASTER_CLOCK / 2;

#define HTOTAL          ((32+8+4+1) * 8)
#define HBEND           (0)
#define HBSTART         (256)

#define VTOTAL          (256+8+4)
#define VBEND           (0)
#define VBSTART         (240)


class quizshow_state : public driver_device
{
public:
	quizshow_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_main_ram(*this, "main_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_cass(*this, "cassette"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	ioport_value tape_headpos_r();
	DECLARE_INPUT_CHANGED_MEMBER(category_select);
	void init_quizshow();
	void quizshow(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<s2650_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_shared_ptr<uint8_t> m_main_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cass;
	output_finder<11> m_lamps;

	void mem_map(address_map &map) ATTR_COLD;

	void lamps1_w(uint8_t data);
	void lamps2_w(uint8_t data);
	void lamps3_w(uint8_t data);
	void tape_control_w(uint8_t data);
	void audio_w(uint8_t data);
	void video_disable_w(uint8_t data);
	uint8_t timing_r();
	int tape_signal_r();
	void flag_output_w(int state);
	void main_ram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void quizshow_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(clock_timer_cb);

	tilemap_t *m_tilemap = nullptr;
	uint32_t m_clocks = 0;
	int m_blink_state = 0;
	int m_category_enable = 0;
	int m_tape_head_pos = 0;
};


/***************************************************************************

  Video

***************************************************************************/

void quizshow_state::quizshow_palette(palette_device &palette) const
{
	palette.set_indirect_color(0, rgb_t::black());
	palette.set_indirect_color(1, rgb_t::white());

	// normal, blink/off, invert, blink+invert
	constexpr int lut_pal[16] = {
		0, 0, 1, 0,
		0, 0, 0, 0,
		1, 0, 0, 0,
		1, 0, 1, 0
	};

	for (int i = 0; i < 16 ; i++)
		palette.set_pen_indirect(i, lut_pal[i]);
}

TILE_GET_INFO_MEMBER(quizshow_state::get_tile_info)
{
	uint8_t const code = m_main_ram[tile_index];

	// d6: blink, d7: invert
	uint8_t const color = (code & (m_blink_state | 0x80)) >> 6;

	tileinfo.set(0, code & 0x3f, color, 0);
}

void quizshow_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizshow_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 16, 32, 16);
}

uint32_t quizshow_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

void quizshow_state::lamps1_w(uint8_t data)
{
	// d0-d3: P1 answer button lamps
	for (int i = 0; i < 4; i++)
		m_lamps[i] = BIT(data, i);

	// d4-d7: N/C
}

void quizshow_state::lamps2_w(uint8_t data)
{
	// d0-d3: P2 answer button lamps
	for (int i = 0; i < 4; i++)
		m_lamps[i + 4] = BIT(data, i);

	// d4-d7: N/C
}

void quizshow_state::lamps3_w(uint8_t data)
{
	// d0-d1: start button lamps
	m_lamps[8] = BIT(data, 0);
	m_lamps[9] = BIT(data, 1);

	// d2-d3: unused? (chip is shared with tape_control_w)
	// d4-d7: N/C
}

void quizshow_state::tape_control_w(uint8_t data)
{
	// d2: enable user category select (changes tape head position)
	m_lamps[10] = BIT(data, 2);
	m_category_enable = (data & 0xc) == 0xc;

	// d3: tape motor
	m_cass->set_motor(BIT(data, 3));

	// d0-d1: unused? (chip is shared with lamps3_w)
	// d4-d7: N/C
}

void quizshow_state::audio_w(uint8_t data)
{
	// d1: audio out
	m_dac->write(BIT(data, 1));

	// d0, d2-d7: N/C
}

void quizshow_state::video_disable_w(uint8_t data)
{
	// d0: video disable (looked glitchy when I implemented it, maybe there's more to it)
	// d1-d7: N/C
}

uint8_t quizshow_state::timing_r()
{
	uint8_t ret = 0x80;

	// d0-d3: 1R-8R (16-line counter)
	ret |= m_clocks >> 1 & 0xf;

	// d4: 8VAC?, use 8V instead
	ret |= m_clocks << 4 & 0x10;

	// d5-d6: 4F-8F
	ret |= m_clocks >> 2 & 0x60;

	// d7: display busy/idle, during in-between tilerows(?) and blanking
	if (m_screen->vpos() >= VBSTART || (m_screen->vpos() + 4) & 8)
		ret &= 0x7f;

	return ret;
}

int quizshow_state::tape_signal_r()
{
	return (m_cass->input() > 0.0) ? 1 : 0;
}

void quizshow_state::flag_output_w(int state)
{
	logerror("Flag output: %d\n", state);
}

void quizshow_state::main_ram_w(offs_t offset, uint8_t data)
{
	m_main_ram[offset]=data;
	m_tilemap->mark_tile_dirty(offset);
}


void quizshow_state::mem_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0bff).rom();
	map(0x1802, 0x1802).w(FUNC(quizshow_state::audio_w));
	map(0x1804, 0x1804).w(FUNC(quizshow_state::lamps1_w));
	map(0x1808, 0x1808).w(FUNC(quizshow_state::lamps2_w));
	map(0x1810, 0x1810).w(FUNC(quizshow_state::lamps3_w));
	map(0x1820, 0x1820).w(FUNC(quizshow_state::tape_control_w));
	map(0x1840, 0x1840).w(FUNC(quizshow_state::video_disable_w));
	map(0x1881, 0x1881).portr("IN0");
	map(0x1882, 0x1882).portr("IN1");
	map(0x1884, 0x1884).portr("IN2");
	map(0x1888, 0x1888).portr("IN3");
	map(0x1900, 0x1900).r(FUNC(quizshow_state::timing_r));
	map(0x1e00, 0x1fff).ram().w(FUNC(quizshow_state::main_ram_w)).share("main_ram");
}


/***************************************************************************

  Inputs

***************************************************************************/

ioport_value quizshow_state::tape_headpos_r()
{
	return 1 << m_tape_head_pos;
}

INPUT_CHANGED_MEMBER(quizshow_state::category_select)
{
	if (newval)
	{
		if (m_category_enable)
			m_tape_head_pos = (m_tape_head_pos + 1) & 3;
	}
}

static INPUT_PORTS_START( quizshow )
	PORT_START("IN0") // ADR strobe 0
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(quizshow_state, tape_headpos_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("IN1") // ADR strobe 1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Answer A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Answer B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Answer C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Answer D")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Answer A") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Answer B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Answer C") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Answer D") PORT_PLAYER(2)

	PORT_START("IN2") // ADR strobe 2
	PORT_DIPNAME( 0x0f, 0x05, "Game Duration" )         PORT_DIPLOCATION("SW3:4,3,2,1")
	PORT_DIPSETTING( 0x00, "50 sec. / 5 questions" )
	PORT_DIPSETTING( 0x01, "60 sec. / 6 questions" )
	PORT_DIPSETTING( 0x02, "70 sec. / 7 questions" )
	PORT_DIPSETTING( 0x03, "80 sec. / 8 questions" )
	PORT_DIPSETTING( 0x04, "90 sec. / 9 questions" )
	PORT_DIPSETTING( 0x05, "100 sec. / 10 questions" )
	PORT_DIPSETTING( 0x06, "110 sec. / 11 questions" )
	PORT_DIPSETTING( 0x07, "120 sec. / 12 questions" )
	PORT_DIPSETTING( 0x08, "130 sec. / 13 questions" )
	PORT_DIPSETTING( 0x09, "140 sec. / 14 questions" )
	PORT_DIPSETTING( 0x0a, "150 sec. / 15 questions" ) // not listed in manual
	PORT_DIPSETTING( 0x0b, "160 sec. / 16 questions" ) // "
	PORT_DIPSETTING( 0x0c, "170 sec. / 17 questions" ) // "
	PORT_DIPSETTING( 0x0d, "180 sec. / 18 questions" ) // "
	PORT_DIPSETTING( 0x0e, "190 sec. / 19 questions" ) // "
	PORT_DIPSETTING( 0x0f, "200 sec. / 20 questions" ) // "

	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, "Duration Mode" )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Question Count" )
	PORT_DIPSETTING(    0x20, "Timed" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // N/C

	PORT_START("IN3") // ADR strobe 3
	PORT_DIPNAME( 0x0f, 0x05, "Bonus Questions" )       PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x01, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "3" )
	PORT_DIPSETTING( 0x04, "4" )
	PORT_DIPSETTING( 0x05, "5" )
	PORT_DIPSETTING( 0x06, "6" )
	PORT_DIPSETTING( 0x07, "7" )
	PORT_DIPSETTING( 0x08, "8" )
	PORT_DIPSETTING( 0x09, "9" )
	PORT_DIPSETTING( 0x0a, "10" ) // not listed in manual
	PORT_DIPSETTING( 0x0b, "11" ) // "
	PORT_DIPSETTING( 0x0c, "12" ) // "
	PORT_DIPSETTING( 0x0d, "13" ) // "
	PORT_DIPSETTING( 0x0e, "14" ) // "
	PORT_DIPSETTING( 0x0f, "15" ) // "
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CAT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Category Select") PORT_CHANGED_MEMBER(DEVICE_SELF, quizshow_state, category_select, 0)

INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout tile_layout =
{
	8, 16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
	},
	8*16
};

static GFXDECODE_START( gfx_quizshow )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 4 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(quizshow_state::clock_timer_cb)
{
	m_clocks++;

	// blink is on 4F and 8F
	int blink_old = m_blink_state;
	m_blink_state = (m_clocks >> 2 & m_clocks >> 1) & 0x40;
	if (m_blink_state != blink_old)
		m_tilemap->mark_all_dirty();
}

void quizshow_state::machine_reset()
{
	m_category_enable = 0;
	m_tape_head_pos = 0;
	m_clocks = 0;
}

void quizshow_state::quizshow(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, MASTER_CLOCK / 16); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &quizshow_state::mem_map);
	m_maincpu->sense_handler().set(FUNC(quizshow_state::tape_signal_r));
	m_maincpu->flag_handler().set(FUNC(quizshow_state::flag_output_w));

	TIMER(config, "clock_timer").configure_periodic(FUNC(quizshow_state::clock_timer_cb), attotime::from_hz(PIXEL_CLOCK / (HTOTAL * 8))); // 8V

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(quizshow_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_quizshow);
	PALETTE(config, m_palette, FUNC(quizshow_state::quizshow_palette), 8*2, 2);

	// sound hardware (discrete)
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED);
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( quizshow )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "005464-01.a1", 0x00000, 0x0200, CRC(c9da809a) SHA1(0d16e552398069a4389c34cc9fb6dcc89eb05b9b) )
	ROM_LOAD( "005464-02.c1", 0x00200, 0x0200, CRC(42237134) SHA1(2932d4820f6c9a383cb5a4e504e043e2d479d474) )
	ROM_LOAD( "005464-03.d1", 0x00400, 0x0200, CRC(0c58fee9) SHA1(c7b081bc4f274a29eb758c8758877b15c9e54d79) )
	ROM_LOAD( "005464-04.f1", 0x00600, 0x0200, CRC(4c6cffd4) SHA1(c291d0fa140faa78b807af72c677d53c620b3103) )
	ROM_LOAD( "005464-05.h1", 0x00800, 0x0200, CRC(b8d61b96) SHA1(eb437a5deaf2fc2a9acebbc506321f3151b4eafa) )
	ROM_LOAD( "005464-06.k1", 0x00a00, 0x0200, CRC(200023b2) SHA1(271d0b2b2f985a6c7b7146869ed00990a52dd653) )

	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASEFF )

	ROM_REGION( 0x0200, "user1", 0 ) // gfx1
	ROM_LOAD_NIB_HIGH( "005466-01.m2", 0x0000, 0x0200, CRC(03017820) SHA1(fd118aa706bdc6976e527ed63388fad01e66270e) )
	ROM_LOAD_NIB_LOW ( "005466-02.n2", 0x0000, 0x0200, CRC(cd554367) SHA1(04da83eb6e2f86f88a3495072b98fbdaca485ae8) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "005465-01.f2", 0x0000, 0x0200, CRC(0fe46552) SHA1(d79b1ff0abfaba1ef2d564d1166c3696e0a1a3f1) ) // memory timing
ROM_END


void quizshow_state::init_quizshow()
{
	uint8_t *gfxdata = memregion("user1")->base();
	uint8_t *dest = memregion("gfx1")->base();

	// convert gfx data to 8*16(actually 8*12), and 2bpp for masking inverted colors
	for (int tile = 0; tile < 0x40; tile++)
	{
		for (int line = 2; line < 14; line ++)
		{
			dest[tile << 4 | line] = 0;
			dest[tile << 4 | line | 0x400] = 0;

			if (line >= 4 && line < 12)
				dest[tile << 4 | line] = gfxdata[(tile ^ 0x3f) << 3 | (line - 4)];
		}
	}
}

} // Anonymous namespace


GAMEL( 1976, quizshow, 0, quizshow, quizshow, quizshow_state, init_quizshow, ROT0, "Atari (Kee Games)", "Quiz Show", MACHINE_NOT_WORKING, layout_quizshow )
