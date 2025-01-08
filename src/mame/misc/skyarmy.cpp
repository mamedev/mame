// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Ryan Holtz
/*
 2010.04.05. stephh

    - Fixed Dip Switches and Inputs (after verification of the Z80 code)
    - Updated memory map to partially handle screen flipping

 05/01/2003  Ryan Holtz
    - Added first AY's status read
    - Added coinage DIP

 2003.01.01. Tomasz Slanina

  changes :
    - nmi generation (incorrect freq probably)
    - music/sfx (partially)
    - more sprite tiles (twice than before)
    - fixed sprites flips
    - scrolling (2nd game level)
    - better colors (weird 'hack' .. but works in most cases ( comparing with screens from emustatus ))
    - dips - lives
    - visible area .. a bit smaller (at least bg 'generation' is not visible for scrolling levels)
    - cpu clock .. now 4 mhz
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class skyarmy_state : public driver_device
{
public:
	skyarmy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram")
	{ }

	void skyarmy(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollram;

	tilemap_t* m_tilemap = nullptr;
	int m_nmi = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void coin_counter_w(int state);
	void nmi_enable_w(int state);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void skyarmy_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(nmi_source);
	void skyarmy_io_map(address_map &map) ATTR_COLD;
	void skyarmy_map(address_map &map) ATTR_COLD;
};

void skyarmy_state::machine_start()
{
	save_item(NAME(m_nmi));
}

TILE_GET_INFO_MEMBER(skyarmy_state::get_tile_info)
{
	int code = m_videoram[tile_index];
	int attr = bitswap<3>(m_colorram[tile_index], 0, 1, 2);

	tileinfo.set(0, code, attr, 0);
}

void skyarmy_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void skyarmy_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void skyarmy_state::skyarmy_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void skyarmy_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skyarmy_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap->set_scroll_cols(32);
}


uint32_t skyarmy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 0x20; i++)
		m_tilemap->set_scrolly(i, m_scrollram[i]);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (int offs = 0; offs < 0x40; offs += 4)
	{
		int const pal = bitswap<3>(m_spriteram[offs + 2], 0, 1, 2);

		int sx = m_spriteram[offs + 3];
		int sy = 240 - (m_spriteram[offs] + 1);
		int flipy = BIT(m_spriteram[offs + 1], 7);
		int flipx = BIT(m_spriteram[offs + 1], 6);

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				m_spriteram[offs + 1] & 0x3f,
				pal,
				flipx, flipy,
				sx, sy, 0);
	}

	return 0;
}

INTERRUPT_GEN_MEMBER(skyarmy_state::nmi_source)
{
	if (m_nmi)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void skyarmy_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


void skyarmy_state::nmi_enable_w(int state)
{
	m_nmi = state;
	if (!m_nmi)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


void skyarmy_state::skyarmy_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8fff).ram().w(FUNC(skyarmy_state::videoram_w)).share("videoram");
	map(0x9000, 0x93ff).ram().w(FUNC(skyarmy_state::colorram_w)).share("colorram");
	map(0x9800, 0x983f).ram().share("spriteram");
	map(0x9840, 0x985f).ram().share("scrollram");
	map(0xa000, 0xa000).portr("DSW");
	map(0xa001, 0xa001).portr("P1");
	map(0xa002, 0xa002).portr("P2");
	map(0xa003, 0xa003).portr("SYSTEM");
	map(0xa000, 0xa007).w("latch", FUNC(ls259_device::write_d0));
}

void skyarmy_state::skyarmy_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay0", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x02).r("ay0", FUNC(ay8910_device::data_r));
	map(0x04, 0x05).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x06, 0x06).r("ay1", FUNC(ay8910_device::data_r));
}


/* verified from Z80 code */
static INPUT_PORTS_START( skyarmy )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, DEF_STR ( Infinite ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x0c, "40000" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	256,
	2,
	{ 0, 256*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	32*2,
	2,
	{ 0, 256*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8 },
	32*8
};

static GFXDECODE_START( gfx_skyarmy )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 8 )
GFXDECODE_END

void skyarmy_state::skyarmy(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &skyarmy_state::skyarmy_map);
	m_maincpu->set_addrmap(AS_IO, &skyarmy_state::skyarmy_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(skyarmy_state::irq0_line_hold));
	m_maincpu->set_periodic_int(FUNC(skyarmy_state::nmi_source), attotime::from_hz(650));

	ls259_device &latch(LS259(config, "latch")); // 11C
	latch.q_out_cb<0>().set(FUNC(skyarmy_state::coin_counter_w));
	latch.q_out_cb<4>().set(FUNC(skyarmy_state::nmi_enable_w)); // ???
	latch.q_out_cb<5>().set(FUNC(skyarmy_state::flip_screen_x_set));
	latch.q_out_cb<6>().set(FUNC(skyarmy_state::flip_screen_y_set));
	latch.q_out_cb<7>().set_nop(); // video RAM buffering?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8,32*8);
	screen.set_visarea(0*8,32*8-1,1*8,31*8-1);
	screen.set_screen_update(FUNC(skyarmy_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skyarmy);
	PALETTE(config, m_palette, FUNC(skyarmy_state::skyarmy_palette), 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, "ay0", 2500000).add_route(ALL_OUTPUTS, "mono", 0.15);
	AY8910(config, "ay1", 2500000).add_route(ALL_OUTPUTS, "mono", 0.15);
}


ROM_START( skyarmy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1h.bin", 0x0000, 0x2000, CRC(46507488) SHA1(a9a43caa6f3eab48c3fd9e2f34fe2b4aacfd4641) )
	ROM_LOAD( "a2h.bin", 0x2000, 0x2000, CRC(0417653e) SHA1(4f6ad7335b5b7e85b4e16cce3c127488c02401b2) )
	ROM_LOAD( "a3h.bin", 0x4000, 0x2000, CRC(95485e56) SHA1(c4cbcd31ba68769d2d0d0875e2a92982265339ae) )
	ROM_LOAD( "j4.bin",  0x6000, 0x2000, CRC(843783df) SHA1(256d8375a8af7de080d456dbc6290a22473d011b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "13b.bin", 0x0000, 0x0800, CRC(3b0e0f7c) SHA1(2bbba10121d3e745146f50c14dc6df97de40fb96) )
	ROM_LOAD( "15b.bin", 0x0800, 0x0800, CRC(5ccfd782) SHA1(408406ae068e5578b8a742abed1c37dcd3720fe5) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "8b.bin",  0x0000, 0x0800, CRC(6ac6bd98) SHA1(e653d80ec1b0f8e07821ea781942dae3de7d238d) )
	ROM_LOAD( "10b.bin", 0x0800, 0x0800, CRC(cada7682) SHA1(83ce8336274cb8006a445ac17a179d9ffd4d6809) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a6.bin",  0x0000, 0x0020, CRC(c721220b) SHA1(61b3320fb616c0600d56840cb6438616c7e0c6eb) )
ROM_END

} // anonymous namespace


GAME( 1982, skyarmy, 0, skyarmy, skyarmy, skyarmy_state, empty_init, ROT90, "Shoei", "Sky Army", MACHINE_SUPPORTS_SAVE )
