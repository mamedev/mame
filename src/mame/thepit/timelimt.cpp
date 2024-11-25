// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi

/***************************************************************************

Time Limit (c) 1983 Chuo
Progress   (c) 1984 Chuo

driver by Ernesto Corvi

Notes:
- Sprite colors are wrong (missing colortable?)
- driver should probably be merged with venture/suprridr.cpp and
  thepit/thepit.cpp
- unused color bank for tilemaps? (colors 0x10-0x1f & 0x30-0x3f)

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class timelimt_state : public driver_device
{
public:
	timelimt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void timelimt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	bool m_nmi_enabled = false;
	bool m_nmi_state = false;
	uint16_t m_scrollx = 0;
	uint8_t m_scrolly = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void nmi_enable_w(int state);
	void coin_lockout_w(int state);

	void videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void scroll_x_lsb_w(uint8_t data);
	void scroll_x_msb_w(uint8_t data);
	void scroll_y_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(main_nmi);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Time Limit has three 32 bytes palette PROM, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void timelimt_state::palette(palette_device &palette) const
{
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	uint8_t const *const color_prom = memregion("proms")->base();

	double weights_r[3], weights_g[3], weights_b[2];
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  0,  0,
			3,  resistances_rg, weights_g,  0,  0,
			2,  resistances_b,  weights_b,  0,  0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_r, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_g, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

TILE_GET_INFO_MEMBER(timelimt_state::get_bg_tile_info)
{
	tileinfo.set(1, m_bg_videoram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(timelimt_state::get_fg_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index], 0, 0);
}

void timelimt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(timelimt_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(timelimt_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_scrollx = 0;
	m_scrolly = 0;

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

/***************************************************************************/

void timelimt_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void timelimt_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void timelimt_state::scroll_x_lsb_w(uint8_t data)
{
	m_scrollx &= 0x100;
	m_scrollx |= data;
}

void timelimt_state::scroll_x_msb_w(uint8_t data)
{
	m_scrollx &= 0xff;
	m_scrollx |= (data & 1) << 8;
}

void timelimt_state::scroll_y_w(uint8_t data)
{
	m_scrolly = data;
}


void timelimt_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int const sy = 240 - m_spriteram[offs];
		int const sx = m_spriteram[offs + 3];
		int code = m_spriteram[offs + 1] & 0x3f;
		int const attr = m_spriteram[offs + 2];
		int const flipy = m_spriteram[offs + 1] & 0x80;
		int const flipx = m_spriteram[offs + 1] & 0x40;

		code += (attr & 0x80) ? 0x40 : 0x00;
		code += (attr & 0x40) ? 0x80 : 0x00;

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
				code,
				attr & 3, // was & 7, wrong for 3bpp and 32 colors
				flipx, flipy,
				sx, sy, 0);
	}
}


uint32_t timelimt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx);
	m_bg_tilemap->set_scrolly(0, m_scrolly);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***************************************************************************/

void timelimt_state::machine_start()
{
	m_nmi_state = false;
	m_nmi_enabled = false;

	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_enabled));
}

void timelimt_state::nmi_enable_w(int state)
{
	m_nmi_enabled = bool(state);
}

void timelimt_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}

/***************************************************************************/

void timelimt_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(timelimt_state::videoram_w)).share(m_videoram);
	map(0x9000, 0x97ff).ram().w(FUNC(timelimt_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x9800, 0x98ff).ram().share(m_spriteram);
	map(0xa000, 0xa000).portr("INPUTS");
	map(0xa800, 0xa800).portr("SYSTEM");
	map(0xb000, 0xb000).portr("DSW");
	map(0xb000, 0xb007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xb800, 0xb800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xb800, 0xb800).nopr();     // NMI ack?
	map(0xc800, 0xc800).w(FUNC(timelimt_state::scroll_x_lsb_w));
	map(0xc801, 0xc801).w(FUNC(timelimt_state::scroll_x_msb_w));
	map(0xc802, 0xc802).w(FUNC(timelimt_state::scroll_y_w));
	map(0xc803, 0xc803).nopw();        // ???? bit 0 used only
	map(0xc804, 0xc804).nopw();        // ???? not used
}

void timelimt_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void timelimt_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3800, 0x3bff).ram();
}

void timelimt_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
	map(0x8c, 0x8d).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x8e, 0x8f).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
}

/***************************************************************************/

static INPUT_PORTS_START( timelimt )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  // probably bonus
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )  // probably screen-flip
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( progress )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // probably unused

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20,000 & 100,000" )
	PORT_DIPSETTING(    0x20, "50,000 & 200,000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) // Manual shows "SCREEN" Table=On / Upright=Off
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2)+0, RGN_FRAC(0,2)+4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_timelimt )
	GFXDECODE_ENTRY( "tiles_1", 0, charlayout,   32, 1 )    // seems correct
	GFXDECODE_ENTRY( "tiles_2", 0, charlayout,    0, 1 )    // seems correct
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 64, 4 )    // seems correct
GFXDECODE_END

/***************************************************************************/

INTERRUPT_GEN_MEMBER(timelimt_state::main_nmi)
{
	m_nmi_state = !m_nmi_state;

	if (m_nmi_enabled && m_nmi_state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else if (!m_nmi_state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

/***************************************************************************/

void timelimt_state::timelimt(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5'000'000);   // 5.000 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &timelimt_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &timelimt_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(timelimt_state::main_nmi));

	Z80(config, m_audiocpu, 18'432'000 / 6);    // 3.072 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &timelimt_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &timelimt_state::sound_io_map);
	m_audiocpu->set_vblank_int("screen", FUNC(timelimt_state::irq0_line_hold)); // ?

	config.set_maximum_quantum(attotime::from_hz(3000));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // IC15
	mainlatch.q_out_cb<0>().set(FUNC(timelimt_state::nmi_enable_w));
	mainlatch.q_out_cb<2>().set(FUNC(timelimt_state::coin_lockout_w));
	mainlatch.q_out_cb<3>().set_nop(); // PSG mute control on thepit not used properly in timelimt (only set at startup, reset at game over)
	mainlatch.q_out_cb<6>().set_nop(); // probably horizontal flip, as on thepit
	mainlatch.q_out_cb<7>().set_nop(); // probably vertical flip, as on thepit

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(timelimt_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_timelimt);
	PALETTE(config, m_palette, FUNC(timelimt_state::palette), 64+32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", 18'432'000 / 12).add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 18'432'000 / 12));
	ay2.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}

/***************************************************************************

  Game ROM(s)

***************************************************************************/

ROM_START( timelimt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "t8",     0x0000, 0x2000, CRC(006767ca) SHA1(a5d528c58cd73c0101ffa9ab783ec870668256db) )
	ROM_LOAD( "t7",     0x2000, 0x2000, CRC(cbe7cd86) SHA1(502a78c14c9717a466ea24cdc63da4c0f3bec1f9) )
	ROM_LOAD( "t6",     0x4000, 0x2000, CRC(f5f17e39) SHA1(7d78f551ce73276725c349703a790f2a63bb5503) )
	ROM_LOAD( "t9",     0x6000, 0x2000, CRC(2d72ab45) SHA1(01d4afacc01b9e7c49355123efd5f5ad4d79a9cd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tl5",    0x0000, 0x1000, CRC(5b782e4a) SHA1(2f4fe2beb8efa5a636fefc1ee172d0200d1c9497) )
	ROM_LOAD( "tl4",    0x1000, 0x1000, CRC(a32883a9) SHA1(26e1725b67be87db28855672facb1504b8ac84d6) )

	ROM_REGION( 0x2000, "tiles_1", 0 )
	ROM_LOAD( "tl11",   0x0000, 0x1000, CRC(46676307) SHA1(38fe80722972b6b3ba32705469a0dcb868fb76a9) )
	ROM_LOAD( "tl10",   0x1000, 0x1000, CRC(2336908a) SHA1(345fc209ce891cc6f8f111c6d3a9e0f65ee6d818) )

	ROM_REGION( 0x2000, "tiles_2", 0 )
	ROM_LOAD( "tl13",   0x0000, 0x1000, CRC(072e4053) SHA1(209edf7b371078e38d1c2812fa6a3d1a78193b3f) )
	ROM_LOAD( "tl12",   0x1000, 0x1000, CRC(ce960389) SHA1(57ee52cfa1b5a3832b362b38c8b7aa411dfc782b) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "tl3",    0x4000, 0x2000, CRC(01a9fd95) SHA1(cd1078700c97a3539c9d9447c55efbd27540a1b3) )
	ROM_LOAD( "tl2",    0x2000, 0x2000, CRC(4693b849) SHA1(fbebedde53599fb1eaedc648bd704b321ab096b5) )
	ROM_LOAD( "tl1",    0x0000, 0x2000, CRC(c4007caf) SHA1(ae05af3319545d5ca98a046bfc100138a5a3ed96) )

	ROM_REGION( 0x0060, "proms", 0 ) // N82S123N color PROMs
	ROM_LOAD( "clr.35", 0x0000, 0x0020, CRC(9c9e6073) SHA1(98496175bf19a8cdb0018705bc1a2193b8a782e1) )
	ROM_LOAD( "clr.48", 0x0020, 0x0020, CRC(a0bcac59) SHA1(e5832831b21981363509b79d89766757bd9273b0) ) // FIXED BITS (xxxxxx1x)
	ROM_LOAD( "clr.57", 0x0040, 0x0020, CRC(3a9f5394) SHA1(0b501f81ce1df722cf7ef982c03e0be337bfe9ee) )
ROM_END

ROM_START( progress )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pg8.bin",     0x0000, 0x2000, CRC(e8779658) SHA1(3eca574d7328d54e544e663f58be789dbf151e77) )
	ROM_LOAD( "pg7.bin",     0x2000, 0x2000, CRC(5dcf6b6f) SHA1(550f02ff5ed2935f4c3c9055c5742fea46f42351) )
	ROM_LOAD( "pg6.bin",     0x4000, 0x2000, CRC(f21d2a08) SHA1(b2542e895d6d011895abec641b056ad8d7dc0d15) )
	ROM_LOAD( "pg9.bin",     0x6000, 0x2000, CRC(052ab4ac) SHA1(a2bfb575f2dfde862f9b1e8a4378f9b6b6200831) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pg4.bin",    0x0000, 0x1000, CRC(b1cc2fe8) SHA1(c9045e7b65311b052c337ad3bedadf108d1c24c3) )

	ROM_REGION( 0x2000, "tiles_1", 0 )
	ROM_LOAD( "pg11.bin",   0x0000, 0x1000, CRC(bd8462e4) SHA1(91b1bd2d69aa1b1a84ee8e642b2c1131a7697dd9) )
	ROM_LOAD( "pg10.bin",   0x1000, 0x1000, CRC(c4bbf0b8) SHA1(d149eda9637474febdafd565a60eb2940702f162) )

	ROM_REGION( 0x2000, "tiles_2", 0 )
	ROM_LOAD( "pg13.bin",   0x0000, 0x1000, CRC(25ec45be) SHA1(1271b7a5632934a82ccae35de8c2968247a233bb) )
	ROM_LOAD( "pg12.bin",   0x1000, 0x1000, CRC(c837c5f5) SHA1(dbfc0d8afe0a8e9dd213cb4095b23b7aa8e2b6f4) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "pg1.bin",    0x0000, 0x2000, CRC(155c8f7f) SHA1(0d32ebceb9b2a0b3faf1f91b7a6800999889b331) )
	ROM_LOAD( "pg2.bin",    0x2000, 0x2000, CRC(a6ca4dfc) SHA1(4243c9ea98e365bf342cf928ff97cafb35cdc7b6) )
	ROM_LOAD( "pg3.bin",    0x4000, 0x2000, CRC(2b21c2fb) SHA1(8c95889a19057d32790c9ccddc0977980eddbd0e) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "35.bin", 0x0000, 0x0020, CRC(8c5ca730) SHA1(be2554e1aa4a74d976919e2c37bce5fb4d40352b) )
	ROM_LOAD( "48.bin", 0x0020, 0x0020, CRC(12dd62cd) SHA1(8322b02d73c3eb44b587f76daeaabe6beea58456) )
	ROM_LOAD( "57.bin", 0x0040, 0x0020, CRC(18455a79) SHA1(e4d64368560e3116a922588129f5f91a4c520f7d) )
ROM_END

} // anonymous namespace


GAME( 1983, timelimt, 0, timelimt, timelimt, timelimt_state, empty_init, ROT90, "Chuo Co. Ltd", "Time Limit", MACHINE_SUPPORTS_SAVE )
GAME( 1984, progress, 0, timelimt, progress, timelimt_state, empty_init, ROT90, "Chuo Co. Ltd", "Progress",   MACHINE_SUPPORTS_SAVE )
