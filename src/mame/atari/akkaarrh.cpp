// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Akka Arrh hardware

    driver by Aaron Giles

    Games supported:
        * Akka Arrh (1982)

    Known issues:
        * Default earam has corrupt Top 3 scores showing upon first use.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/er2055.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"

#include "akkaarrh.lh"


namespace {

class akkaarrh_state : public driver_device
{
public:
	akkaarrh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_earom(*this, "earom"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_palette(*this, "palette"),
		m_palette_ram(*this, "paletteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void akkaarrh(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_tile_info);

	void videoram_w(offs_t offset, uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	void irq_ack_w(uint8_t data);
	void output0_w(uint8_t data);
	void output1_w(uint8_t data);
	void output2_w(uint8_t data);
	void output3_w(uint8_t data);
	void video_mirror_w(uint8_t data);
	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<er2055_device> m_earom;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_palette_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	output_finder<26> m_lamps;

	tilemap_t * m_tilemap[4]{};
	uint8_t m_video_mirror = 0;
};

static constexpr XTAL MASTER_CLOCK = 12.096_MHz_XTAL;



/*************************************
 *
 *  Video system start and update
 *
 *************************************/

void akkaarrh_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akkaarrh_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 30);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akkaarrh_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 30);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akkaarrh_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 30);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akkaarrh_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 30);

	m_tilemap[0]->set_flip(0);
	m_tilemap[1]->set_flip(TILEMAP_FLIPX);
	m_tilemap[2]->set_flip(TILEMAP_FLIPY);
	m_tilemap[3]->set_flip(TILEMAP_FLIPX | TILEMAP_FLIPY);

	m_lamps.resolve();
}

uint32_t akkaarrh_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw the background
	if ((m_video_mirror & 1) == 0)
	{
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		// the zoom-effect mirrors the upper-left quadrant
		static const rectangle quadrants[4] =
		{
			{   0, 127,   0, 119 },
			{ 128, 255,   0, 119 },
			{   0, 127, 120, 239 },
			{ 128, 255, 120, 239 }
		};

		for (uint32_t i = 0 ; i < 4; ++i)
		{
			rectangle clip = cliprect;
			clip &= quadrants[i];
			m_tilemap[i]->draw(screen, bitmap, clip, 0, 0);
		}
	}

	gfx_element *gfx = m_gfxdecode->gfx(1);

	// draw the sprites
	for (uint32_t offs = 0; offs < 0x100; offs += 4)
	{
		int code = m_spriteram[offs + 1];
		int color = m_spriteram[offs] & 0xf;
		int x = m_spriteram[offs + 3];
		int y = 240 - m_spriteram[offs + 2];

		gfx->transpen(bitmap, cliprect, code, color, 0, 0, x, y, 0);
	}

	return 0;
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

TILE_GET_INFO_MEMBER(akkaarrh_state::get_tile_info)
{
	int data = m_videoram[tile_index];
	int data2 = m_videoram[tile_index + 0x400];
	tileinfo.set(0, data, data2 & 0xf, TILE_FLIPYX(data2 >> 6));
}

void akkaarrh_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;

	int tile = offset & 0x3ff;
	m_tilemap[0]->mark_tile_dirty(tile);
	m_tilemap[1]->mark_tile_dirty(tile);
	m_tilemap[2]->mark_tile_dirty(tile);
	m_tilemap[3]->mark_tile_dirty(tile);
}

void akkaarrh_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_palette_ram[offset] = data;
	m_palette->set_pen_color(offset, pal3bit(data >> 5), pal2bit(data >> 3), pal3bit(data >> 0));
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void akkaarrh_state::video_mirror_w(uint8_t data)
{
	m_video_mirror = data;
}

void akkaarrh_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

void akkaarrh_state::output0_w(uint8_t data)
{
	// 765----- unknown (always 0?)
	// ---4---- unknown (1 in attract mode, 0 when playing)
	// ----3--- player 2 lamp
	// -----2-- player 1 lamp
	// ------1- coin counter 2
	// -------0 coin counter 1

	m_lamps[0] = !BIT(data, 3);
	m_lamps[1] = !BIT(data, 2);

	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}

void akkaarrh_state::output1_w(uint8_t data)
{
	// 7------- lamp 1 left bezel (top)
	// -6------ lamp 2 left bezel
	// --543--- unknown (1 in attract mode, 0 when playing)
	// -----2-- unknown (always 0?)
	// ------1- shooting lamp? toggles when shooting and not zoomed in
	// -------0 bottom lamp

	for (int i = 0; i < 8; i++)
		m_lamps[2 + i] = BIT(data, i);
}

void akkaarrh_state::output2_w(uint8_t data)
{
	// 7------- lamp 3 right bezel
	// -6------ lamp 1 top bezel (left)
	// --5----- lamp 2 top bezel
	// ---4---- lamp 3 top bezel (middle)
	// ----3--- lamp 4 top bezel
	// -----2-- lamp 5 top bezel (right)
	// ------1- lamp 3 left bezel
	// -------0 lamp 4 left bezel (bottom)

	for (int i = 0; i < 8; i++)
		m_lamps[10 + i] = BIT(data, i);
}

void akkaarrh_state::output3_w(uint8_t data)
{
	// 7------- lamp zoomed in
	// -6------ lamp warning
	// --543--- unknown (1 in attract mode, 0 when playing)
	// -----2-- lamp 2 right bezel
	// ------1- lamp 1 right bezel (top)
	// -------0 lamp 4 right bezel (bottom)

	for (int i = 0; i < 8; i++)
		m_lamps[18 + i] = BIT(data, i);
}



/*************************************
 *
 *  High score EAROM
 *
 *************************************/

uint8_t akkaarrh_state::earom_read()
{
	return m_earom->data();
}

void akkaarrh_state::earom_write(offs_t offset, uint8_t data)
{
	m_earom->set_address(offset & 0x3f);
	m_earom->set_data(data);
}

void akkaarrh_state::earom_control_w(uint8_t data)
{
	// CK = DB0, C1 = /DB2, C2 = DB1, CS1 = DB3, /CS2 = GND
	m_earom->set_control(BIT(data, 3), 1, !BIT(data, 2), BIT(data, 1));
	m_earom->set_clk(BIT(data, 0));
}



/*************************************
 *
 *  CPU memory map
 *
 *************************************/

void akkaarrh_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x10ff).ram().share("spriteram");
	map(0x2000, 0x27ff).ram().w(FUNC(akkaarrh_state::videoram_w)).share("videoram");
	map(0x3000, 0x30ff).ram().w(FUNC(akkaarrh_state::paletteram_w)).share("paletteram");
	map(0x4000, 0x4000).w(FUNC(akkaarrh_state::irq_ack_w));
	map(0x5000, 0x5000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x6000, 0x6000).w(FUNC(akkaarrh_state::video_mirror_w));
	map(0x7010, 0x701f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x7020, 0x702f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x7040, 0x707f).nopr().w(FUNC(akkaarrh_state::earom_write));
	map(0x7080, 0x7080).portr("7080");
	map(0x7081, 0x7081).portr("7081");
	map(0x7082, 0x7082).portr("7082");
	map(0x7083, 0x7083).portr("7083");
	map(0x7087, 0x7087).r(FUNC(akkaarrh_state::earom_read));
	map(0x70c0, 0x70c0).w(FUNC(akkaarrh_state::output0_w));
	map(0x70c1, 0x70c1).w(FUNC(akkaarrh_state::output1_w));
	map(0x70c2, 0x70c2).w(FUNC(akkaarrh_state::output2_w));
	map(0x70c3, 0x70c3).w(FUNC(akkaarrh_state::output3_w));
	map(0x70c7, 0x70c7).w(FUNC(akkaarrh_state::earom_control_w));
	map(0x8000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( akkaarrh )
	PORT_START("7080")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Zoom")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Power blaster")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("7081")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    // tested; if 0, writes 240 to $FD
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // resets game
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("7082")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("7083")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_akkaarrh )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x4_planar,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, gfx_16x16x4_planar, 0, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void akkaarrh_state::akkaarrh(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, MASTER_CLOCK/8); // Unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &akkaarrh_state::main_map);
	m_maincpu->set_periodic_int(FUNC(akkaarrh_state::irq0_line_assert), attotime::from_hz(4 * 60));

	WATCHDOG_TIMER(config, "watchdog");
	ER2055(config, m_earom);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_akkaarrh);
	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(256, 262);
	m_screen->set_visarea(0, 255, 0, 239);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(akkaarrh_state::screen_update));
	m_screen->set_palette(m_palette);

	config.set_default_layout(layout_akkaarrh);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", 1250000)); // Unverified
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.5);

	pokey_device &pokey2(POKEY(config, "pokey2", 1250000)); // Unverified
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.5);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( akkaarrh )
	 ROM_REGION( 0x10000, "maincpu", 0 )
	 ROM_LOAD( "akka_8000.p1",  0x8000, 0x1000, CRC(578bb162) SHA1(f003d9a63e397c377e4738ca31a637da1da2cbdb) )
	 ROM_LOAD( "akka_9000.m1",  0x9000, 0x1000, CRC(837fa612) SHA1(2a2ccbf879fd9cfc0342e6c5bbcd23962bdbbcde) )
	 ROM_LOAD( "akka_a000.l1",  0xa000, 0x1000, CRC(13c769e9) SHA1(8e6c7b21fa555850ba8d00abbb4516e483964b5d) )
	 ROM_LOAD( "akka_b000.j1",  0xb000, 0x1000, CRC(35c04f28) SHA1(71ad7eaf5bf96c1a0a321f1b04706afd40d9757f) )
	 ROM_LOAD( "akka_c000.h1",  0xc000, 0x1000, CRC(17e85ac4) SHA1(bd010060eaf8fbf27176d4fdec241ee3b42aece8) )
	 ROM_LOAD( "akka_d000.e1",  0xd000, 0x1000, CRC(03fb4143) SHA1(3a2106d7322139b9924566133d084bb7c5d769bc) )
	 ROM_LOAD( "akka_e000.f1",  0xe000, 0x1000, CRC(8d3e671c) SHA1(c99d92c4afaa1ba043520b98d925f83490e49f0a) )
	 ROM_RELOAD(                0xf000, 0x1000 )

	 ROM_REGION( 0x2000, "gfx1", 0 )
	 ROM_LOAD( "akka_pf0.l6",  0x0000, 0x0800, CRC(5c10b63e) SHA1(27ac6cebb3f6733cb830210f4723938ec2ddaafc) )
	 ROM_LOAD( "akka_pf1.j6",  0x0800, 0x0800, CRC(636fd64c) SHA1(c2985332b8f32a8dd096fc081df12ae1a132e6a3) )
	 ROM_LOAD( "akka_pf2.p6",  0x1000, 0x0800, CRC(a5f25d69) SHA1(8f5af774db3ed3969c8c0a0b530ea62198830e3d) )
	 ROM_LOAD( "akka_pf3.m6",  0x1800, 0x0800, CRC(a3449469) SHA1(42f2ccf6ea02987bfb91ff1ca42684e9700f2f8f) )

	 ROM_REGION( 0x4000, "gfx2", 0 )
	 ROM_LOAD( "akka_mo0.f11", 0x0000, 0x1000, CRC(71bd1bc6) SHA1(9c3d2c039c648834a7a0b98cd2dcbbbb88f74dd8) )
	 ROM_LOAD( "akka_mo1.d11", 0x1000, 0x1000, CRC(a5ee8ecc) SHA1(d2a8c9791dca9157d9d20b8a4f6e760e6db35f05) )
	 ROM_LOAD( "akka_mo2.a11", 0x2000, 0x1000, CRC(11cec4d9) SHA1(32ea614c25473aeb9a012a3980ad616592f6c3bb) )
	 ROM_LOAD( "akka_mo3.b11", 0x3000, 0x1000, CRC(adcf6a36) SHA1(0555a252ba39400d2c533add6ac492926674d6ad) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, akkaarrh, 0, akkaarrh, akkaarrh, akkaarrh_state, empty_init, ROT0, "Atari", "Akka Arrh (prototype)", 0 )
