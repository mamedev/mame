// license:BSD-3-Clause
// copyright-holders:Mike Balfour

/***************************************************************************

    Atari Sky Diver hardware

    driver by Mike Balfour

    Games supported:
        * Sky Diver

    Known issues:
        * There is a problem with coin input not starting when in demo mode.
        * The NMI interrupt needs to be more accurate, to do 32V, adjusted
          to VBLANK.  This also affects sound.
        * The current value of 5 interrupts per frame, works pretty good,
          but is not 100% accurate timing wise.

****************************************************************************

    Memory Map:
    0000-00FF    R/W    PAGE ZERO RAM
    0010         R/W    H POS PLANE 1
    0011         R/W    H POS PLANE 2
    0012         R/W    H POS MAN 1
    0013         R/W    H POS MAN 2
    0014         R/W    RANGE LOAD
    0015         R/W    NOTE LOAD
    0016         R/W    NAM LD
    0017         R/W    UNUSED
    0018         R/W    V POS PLANE 1
    0019         R/W    PICTURE PLANE 1
    001A         R/W    V POS PLANE 2
    001B         R/W    PICTURE PLANE 2
    001C         R/W    V POS MAN 1
    001D         R/W    PICTURE MAN 1
    001E         R/W    V POS MAN 2
    001F         R/W    PICTURE MAN 2
    0400-077F    R/W    PLAYFIELD
    0780-07FF    R/W    MAPS TO 0000-D0
    0800-0801     W     S LAMP
    0802-0803     W     K LAMP
    0804-0805     W     START LITE 1
    0806-0807     W     START LITE 2
    0808-0809     W     Y LAMP
    080A-080B     W     D LAMP
    080C-080D     W     SOUND ENABLE
    1000-1001     W     JUMP LITE 1
    1002-1003     W     COIN LOCK OUT
    1006-1007     W     JUMP LITE 2
    1008-1009     W     WHISTLE 1
    100A-100B     W     WHISTLE 2
    100C-100D     W     NMION
    100E-100F     W     WIDTH
    1800          R     D6=LEFT 1, D7=RIGHT 1
    1801          R     D6=LEFT 2, D7=RIGHT 2
    1802          R     D6=JUMP 1, D7=CHUTE 1
    1803          R     D6=JUMP 2, D7=CHUTE 2
    1804          R     D6=(D) OPT SW: NEXT TEST, D7=(F) OPT SW
    1805          R     D6=(E) OPT SW, D7= (H) OPT SW: DIAGNOSTICS
    1806          R     D6=START 1, D7=COIN 1
    1807          R     D6=START 2, D7=COIN 2
    1808          R     D6=MISSES 2, D7=MISSES 1
    1809          R     D6=COIN 2, D7=COIN1
    180A          R     D6=HARD/EASY, D7=EXTENDED PLAY
    180B          R     D6=LANGUAGE 2, D7=LANGUAGE 1
    1810          R     D6=TEST, D7=!VBLANK
    1811          R     D6=!SLAM, D7=UNUSED
    2000          W     TIMER RESET
    2002-2003     W     I LAMP
    2004-2005     W     V LAMP
    2006-2007     W     E LAMP
    2008-2009     W     R LAMP
    200A-200B     W     OCT 1
    200C-200D     W     OCT 2
    200E-200F     W     NOISE RESET
    2800-2FFF     R     ROM 0
    3000-37FF     R     ROM 1
    3800-3FFF     R     ROM 2A
    7800-7FFF     R     ROM 2B

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    Notes:

    The NMI interrupts are only used to read the coin switches.

***************************************************************************/

#include "emu.h"

#include "skydiver_a.h"

#include "cpu/m6800/m6800.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "skydiver.lh"


namespace {

class skydiver_state : public driver_device
{
public:
	skydiver_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_latch3(*this, "latch3"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram")
	{ }

	void skydiver(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<f9334_device> m_latch3;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	uint8_t m_nmion = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_width = 0;

	void nmion_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t wram_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);
	void width_w(int state);
	void coin_lockout_w(int state);
	void latch3_watchdog_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	void program_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(skydiver_state::get_tile_info)
{
	uint8_t const code = m_videoram[tile_index];
	tileinfo.set(0, code & 0x3f, code >> 6, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void skydiver_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skydiver_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	save_item(NAME(m_nmion));
	save_item(NAME(m_width));
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void skydiver_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


uint8_t skydiver_state::wram_r(offs_t offset)
{
	return m_videoram[offset | 0x380];
}

void skydiver_state::wram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset | 0x0380] = data;
}


void skydiver_state::width_w(int state)
{
	m_width = state;
}


void skydiver_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_global_w(!state);
}


void skydiver_state::latch3_watchdog_w(offs_t offset, uint8_t data)
{
	m_watchdog->watchdog_reset();
	m_latch3->write_a0(offset);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

void skydiver_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw each one of our four motion objects, the two PLANE sprites
	   can be drawn double width */
	for (int pic = 3; pic >= 0; pic--)
	{
		int sx = 29 * 8 - m_videoram[pic + 0x0390];
		int const sy = 30 * 8 - m_videoram[pic * 2 + 0x0398];
		int charcode = m_videoram[pic * 2 + 0x0399];
		int const xflip = charcode & 0x10;
		int const yflip = charcode & 0x08;
		int const wide = (~pic & 0x02) && m_width;
		charcode = (charcode & 0x07) | ((charcode & 0x60) >> 2);
		int const color = pic & 0x01;

		if (wide)
		{
			sx -= 8;
		}

		m_gfxdecode->gfx(1)->zoom_transpen(bitmap, cliprect,
			charcode, color,
			xflip, yflip, sx, sy,
			wide ? 0x20000 : 0x10000, 0x10000, 0);
	}
}


uint32_t skydiver_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Palette generation
 *
 *************************************/

static constexpr unsigned colortable_source[] =
{
	0x02, 0x00,
	0x02, 0x01,
	0x00, 0x02,
	0x01, 0x02
};

void skydiver_state::palette(palette_device &palette) const
{
	constexpr rgb_t colors[]{ rgb_t::black(), rgb_t::white(), rgb_t(0xa0, 0xa0, 0xa0) }; // black, white, grey
	for (unsigned i = 0; i < std::size(colortable_source); i++)
	{
		assert(colortable_source[i] < std::size(colors));
		palette.set_pen_color(i, colors[colortable_source[i]]);
	}
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

void skydiver_state::nmion_w(int state)
{
	m_nmion = state;
}


INTERRUPT_GEN_MEMBER(skydiver_state::interrupt)
{
	// Convert range data to divide value and write to sound
	m_discrete->write(SKYDIVER_RANGE_DATA, (0x01 << (~m_videoram[0x394] & 0x07)) & 0xff);   // Range 0-2

	m_discrete->write(SKYDIVER_RANGE3_EN,  m_videoram[0x394] & 0x08);       // Range 3 - note disable
	m_discrete->write(SKYDIVER_NOTE_DATA, ~m_videoram[0x395] & 0xff);       // Note - freq
	m_discrete->write(SKYDIVER_NOISE_DATA,  m_videoram[0x396] & 0x0f);  // NAM - Noise Amplitude

	if (m_nmion)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void skydiver_state::program_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).mirror(0x4300).rw(FUNC(skydiver_state::wram_r), FUNC(skydiver_state::wram_w));
	map(0x0080, 0x00ff).mirror(0x4000).ram();       // RAM B1
	map(0x0400, 0x07ff).mirror(0x4000).ram().w(FUNC(skydiver_state::videoram_w)).share(m_videoram);       // RAMs K1,M1,P1,J1,N1,K/L1,L1,H/J1
	map(0x0800, 0x080f).mirror(0x47f0).w("latch1", FUNC(f9334_device::write_a0));
	map(0x1000, 0x100f).mirror(0x47f0).w("latch2", FUNC(f9334_device::write_a0));
	map(0x1800, 0x1800).mirror(0x47e0).portr("IN0");
	map(0x1801, 0x1801).mirror(0x47e0).portr("IN1");
	map(0x1802, 0x1802).mirror(0x47e0).portr("IN2");
	map(0x1803, 0x1803).mirror(0x47e0).portr("IN3");
	map(0x1804, 0x1804).mirror(0x47e0).portr("IN4");
	map(0x1805, 0x1805).mirror(0x47e0).portr("IN5");
	map(0x1806, 0x1806).mirror(0x47e0).portr("IN6");
	map(0x1807, 0x1807).mirror(0x47e0).portr("IN7");
	map(0x1808, 0x1808).mirror(0x47e4).portr("IN8");
	map(0x1809, 0x1809).mirror(0x47e4).portr("IN9");
	map(0x180a, 0x180a).mirror(0x47e4).portr("IN10");
	map(0x180b, 0x180b).mirror(0x47e4).portr("IN11");
	map(0x1810, 0x1810).mirror(0x47e4).portr("IN12");
	map(0x1811, 0x1811).mirror(0x47e4).portr("IN13");
	map(0x2000, 0x201f).mirror(0x47e0).r(m_watchdog, FUNC(watchdog_timer_device::reset_r)).w(FUNC(skydiver_state::latch3_watchdog_w));
	map(0x2800, 0x2fff).mirror(0x4000).rom();
	map(0x3000, 0x37ff).mirror(0x4000).rom();
	map(0x3800, 0x3fff).rom();
	map(0x7800, 0x7fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( skydiver )
	PORT_START("IN0")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START("IN1")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )    // Jump 1
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )    // Chute 1

	PORT_START("IN3")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // Jump 2
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // Chute 2

	PORT_START("IN4")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("(D) OPT SW NEXT TEST") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("(F) OPT SW") PORT_CODE(KEYCODE_F)

	PORT_START("IN5")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("(E) OPT SW") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("(H) OPT SW DIAGNOSTICS") PORT_CODE(KEYCODE_H)

	PORT_START("IN6")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("IN7")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("IN8")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("IN9")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("IN10")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN11")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( German ) )

	PORT_START("IN12")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN13")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHISTLE1")
	PORT_ADJUSTER( 33, "Whistle 1 Freq" )

	PORT_START("WHISTLE2")
	PORT_ADJUSTER( 25, "Whistle 2 Freq" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 15, 14, 13, 12 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const gfx_layout motion_layout =
{
	16,16,
	32,
	1,
	{ 0 },
	{ 4, 5, 6, 7, 4 + 0x400*8, 5 + 0x400*8, 6 + 0x400*8, 7 + 0x400*8,
		12, 13, 14, 15, 12 + 0x400*8, 13 + 0x400*8, 14 + 0x400*8, 15 + 0x400*8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	8*32
};


static GFXDECODE_START( gfx_skydiver )
	GFXDECODE_ENTRY( "chars",   0, charlayout,    0, 4 )
	GFXDECODE_ENTRY( "sprites", 0, motion_layout, 0, 4 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void skydiver_state::skydiver(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 12.096_MHz_XTAL / 16);     // ????
	m_maincpu->set_addrmap(AS_PROGRAM, &skydiver_state::program_map);
	m_maincpu->set_periodic_int(FUNC(skydiver_state::interrupt), attotime::from_hz(5*60));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 8);    // 128V clocks the same as VBLANK

	f9334_device &latch1(F9334(config, "latch1")); // F12
	latch1.q_out_cb<0>().set_output("lamps");
	latch1.q_out_cb<1>().set_output("lampk");
	latch1.q_out_cb<2>().set_output("led0"); // start lamp 1
	latch1.q_out_cb<3>().set_output("led1"); // start lamp 2
	latch1.q_out_cb<4>().set_output("lampy");
	latch1.q_out_cb<5>().set_output("lampd");
	latch1.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<SKYDIVER_SOUND_EN>));

	f9334_device &latch2(F9334(config, "latch2")); // H12
	//latch2.q_out_cb<0>().set(FUNC(skydiver_state::jump1_lamps_w));
	latch2.q_out_cb<1>().set(FUNC(skydiver_state::coin_lockout_w));
	//latch2.q_out_cb<3>().set(FUNC(skydiver_state::jump2_lamps_w));
	latch2.q_out_cb<4>().set("discrete", FUNC(discrete_device::write_line<SKYDIVER_WHISTLE1_EN>));
	latch2.q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<SKYDIVER_WHISTLE2_EN>));
	latch2.q_out_cb<6>().set(FUNC(skydiver_state::nmion_w));
	latch2.q_out_cb<7>().set(FUNC(skydiver_state::width_w));

	f9334_device &latch3(F9334(config, "latch3")); // A11
	latch3.q_out_cb<1>().set_output("lampi");
	latch3.q_out_cb<2>().set_output("lampv");
	latch3.q_out_cb<3>().set_output("lampe");
	latch3.q_out_cb<4>().set_output("lampr");
	latch3.q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<SKYDIVER_OCT1_EN>));
	latch3.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<SKYDIVER_OCT2_EN>));
	latch3.q_out_cb<7>().set("discrete", FUNC(discrete_device::write_line<SKYDIVER_NOISE_RST>));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12.096_MHz_XTAL / 2, 384, 0, 256, 262, 0, 224);
	screen.set_screen_update(FUNC(skydiver_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skydiver);
	PALETTE(config, m_palette, FUNC(skydiver_state::palette), std::size(colortable_source));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, skydiver_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( skydiver )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "33167-02.f1", 0x2800, 0x0800, CRC(25a5c976) SHA1(50fbf5dceab5d78292dc14bf25f2076e8139a594) )
	ROM_LOAD( "33164-02.e1", 0x3000, 0x0800, CRC(a348ac39) SHA1(7401cbd2f7236bd1d6ad0e39eb3de2b7d75e8f45) )
	ROM_LOAD( "33165-02.d1", 0x3800, 0x0800, CRC(a1fc5504) SHA1(febaa78936de7703b708c0d1f350fe288e0a106b) )
	ROM_LOAD( "33166-02.c1", 0x7800, 0x0800, CRC(3d26da2b) SHA1(e515d5c13814b9732a6ca109272500a60edc208a) )

	ROM_REGION( 0x0400, "chars", 0 )
	ROM_LOAD( "33163-01.h5", 0x0000, 0x0400, CRC(5b9bb7c2) SHA1(319f45b6dff96739f73f2089361239da47042dcd) )

	ROM_REGION( 0x0800, "sprites", 0 )
	ROM_LOAD( "33176-01.l5", 0x0000, 0x0400, CRC(6b082a01) SHA1(8facc94843ea041d205137056bd2035cf968125b) )
	ROM_LOAD( "33177-01.k5", 0x0400, 0x0400, CRC(f5541af0) SHA1(0967269518b6eac3c4e9ddaee39303086476c580) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL(1978, skydiver, 0, skydiver, skydiver, skydiver_state, empty_init, ROT0, "Atari", "Sky Diver", MACHINE_SUPPORTS_SAVE, layout_skydiver )
