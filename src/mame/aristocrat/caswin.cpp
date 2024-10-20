// license:BSD-3-Clause
// copyright-holders:Chris Hardy, Angelo Salese
/**************************************************************************************************

Royal Casino (c) 1984 Dyna
Casino Winner (c) 1985 Aristocrat

driver by Chris Hardy & Angelo Salese
original rcasino.c driver by Curt Coder

TODO:
- Imperfect 8-liner video mode, which in turn looks derived from misc/summit.cpp?
- Hookup payout (press 'I' to bypass when "call dealer" happens);
- Add lamps support;
- p1 & p2 inputs are tied to the same port...maybe they are mux-ed with the flip screen bit?

===================================================================================================
    ----------------------------------------
    Casino Royal by Dyna Electronics CO. LTD
    ----------------------------------------

    Location    Device      File ID     Checksum
    --------------------------------------------
    18B          2764        RI-W1        C62D
    16B          2764        RI-W2        AC85
    15B          2732        RI-W3        70B7
    11B          2732        RI-W4        0C21
    9B           2764        RI-W5        EB59
    8B           2764        RI-W6        C934
    6B           2732        RI-W7        4130
    9E         82S123      PROM1.BPR      0F29
    8E         82S123      PROM2.BPR      0EE5

    Notes: PCB No. D-2608208A1-2

    Brief hardware overview
    -----------------------

    Main processor  - Z80
    Sound           - AY-3-8910

    ----------------------------------------
    Casino Royal by Dyna Electronics CO. LTD
    ----------------------------------------

    PCB No. D-2608208A1-1 (Has angled corner cut on the pcb next to this identifier.)

    Brief hardware overview
    -----------------------

    Xtal            - 6.000 MHz @ 13L
    Main processor  - Z80A
    Sound           - AY-3-8910

    ----------------------------------------
    Casino Royal by Dyna Electronics CO. LTD
    ----------------------------------------

    PCB No. D-2608208A1-1 (Labeled with Nagoya Japan)

    Brief hardware overview
    -----------------------

    Xtal            - 6.000 MHz @ 2G
    Main processor  - Z80A
    Sound           - AY-3-8910

**************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class caswin_state : public driver_device
{
public:
	caswin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_lamps(*this, "lamp%u", 0U)
		, m_vram(*this, "vram")
		, m_attr(*this, "attr")
	{ }

	void vvillage(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void vram_w(offs_t offset, uint8_t data);
	void attr_w(offs_t offset, uint8_t data);
	void base_scroll_w(uint8_t data);
	void vregs_w(uint8_t data);
	uint8_t rng_r();
	void coin_meters_w(uint8_t data);
	void lamps_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	tilemap_t *m_tilemap = nullptr;
	bool m_window_enable = false;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<5> m_lamps;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_attr;
};



TILE_GET_INFO_MEMBER(caswin_state::get_tile_info)
{
	const u8 attr = m_attr[tile_index];
	const u16 tile = (m_vram[tile_index] | ((attr & 0x70)<<4)) & 0x7ff;
	const u8 colour = m_attr[tile_index] & 0xf;

	// for 8-liner mode all dedicated objects have this high
	tileinfo.category = BIT(attr, 3);

	tileinfo.set(0,
			tile,
			colour,
			0);
}

void caswin_state::video_start()
{
	m_lamps.resolve();
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(caswin_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap->set_transparent_pen(0);
}

uint32_t caswin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_window_enable)
	{
		m_tilemap->set_scroll_rows(1);
		m_tilemap->set_scrollx(0, 0);
		m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
		return 0;
	}
	// 8-liner mode
	rectangle clip;

	bitmap.fill(0, cliprect);

	m_tilemap->set_scroll_rows(1);
	m_tilemap->set_scrollx(0, 0);

	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_CATEGORY(0), 0);
	const rectangle &visarea = screen.visible_area();
	// these limits are calculated thru tilemap viewer against the markers,
	// which shouldn't draw
	clip.set(56, 199, visarea.min_y, visarea.max_y);
	clip &= cliprect;
	m_tilemap->set_scroll_rows(32);

	// TODO: still some cutoffs / object change flashing
	// goofy rowscroll, expects to wraparound over its own canvas ...
	for (int i = 0; i < 32; i ++)
	{
		const u16 scroll_data = m_vram[i];
		m_tilemap->set_scrollx(i, scroll_data ? scroll_data + 0 : 0);
	}

	m_tilemap->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(1), 0);

	for (int i = 0; i < 32; i ++)
	{
		const u16 scroll_data = m_vram[i];
		m_tilemap->set_scrollx(i, scroll_data ? scroll_data + 96 : 0);
	}

	m_tilemap->draw(screen, bitmap, clip, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_CATEGORY(1), 0);
	return 0;
}

void caswin_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void caswin_state::attr_w(offs_t offset, uint8_t data)
{
	m_attr[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

// TODO: bit 5 unknown, acts as flip-flop
// TODO: bits 4-0 gets often changed during reel motion.
void caswin_state::base_scroll_w(uint8_t data)
{
	//...
}

/*---- --x- enable window effect? */
/*---- ---x flip screen */
void caswin_state::vregs_w(uint8_t data)
{
	flip_screen_set(data & 1);
	m_window_enable = bool(BIT(data, 1));
}

/**********************
*
* I/Os
*
**********************/

// TODO: pinpoint actual algo
uint8_t caswin_state::rng_r()
{
	return machine().rand();
}

void caswin_state::coin_meters_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,data & 1);
	machine().bookkeeping().coin_counter_w(1,data & 1);
	// data & 4 payout counter
	machine().bookkeeping().coin_lockout_w(0,data & 0x20);
	machine().bookkeeping().coin_lockout_w(1,data & 0x20);
}

/*
 * ---x ---- lamp button 5
 * ---- x--- lamp button 4
 * ---- -x-- lamp button 3
 * ---- --x- lamp button 2
 * ---- ---x lamp button 1
 */
void caswin_state::lamps_w(uint8_t data)
{
	for (unsigned i = 0; i < 5; i++)
		m_lamps[i] = BIT(data, i);
}

void caswin_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa000).r(FUNC(caswin_state::rng_r)); //accessed by caswin only
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xf000, 0xf3ff).ram().w(FUNC(caswin_state::vram_w)).share("vram");
	map(0xf800, 0xfbff).ram().w(FUNC(caswin_state::attr_w)).share("attr");
}

void caswin_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("IN0");
	map(0x11, 0x11).portr("IN1");
	map(0x10, 0x10).w(FUNC(caswin_state::base_scroll_w));
	map(0x11, 0x11).w(FUNC(caswin_state::vregs_w));
	map(0x12, 0x12).w(FUNC(caswin_state::lamps_w));
	map(0x13, 0x13).w(FUNC(caswin_state::coin_meters_w));
}


static INPUT_PORTS_START( vvillage )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note Acceptor")// Note
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Hopper Payout")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )                        // Hopper Micro
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )                          // 1P FlipFlop
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )                          // 2P FlipFlop
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Poker Available" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Black Jack Available" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Hi & Low Available" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Five Line Available" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Super Conti Available" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "BlackJack, Even Rule" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Poker, Royal Flush Rule" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Poker, Jack or Better Rule") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Enable Hopper Payout" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Hopper Win Payout") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Hi Lo, Royal Flush" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Bet" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Normal Game")
	PORT_DIPSETTING(    0x00, "Double Game")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x80, "Analyzer" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0, 2), RGN_FRAC(1,2) },
	{ STEP8(7, -1) },
	{ STEP8(0, 8) },
	8*8
};

static GFXDECODE_START( gfx_vvillage )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

void caswin_state::palette_init(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x40; ++i)
	{
		int bit0, bit1, bit2;

		bit0 = 0;
		bit1 = (color_prom[0] >> 0) & 0x01;
		bit2 = (color_prom[0] >> 1) & 0x01;
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		bit2 = (color_prom[0] >> 4) & 0x01;
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 5) & 0x01;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


void caswin_state::vvillage(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(6'000'000)); // Z80A @ 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &caswin_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &caswin_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(caswin_state::irq0_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(caswin_state::screen_update));
	screen.set_palette("palette");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_vvillage);
	PALETTE(config, "palette", FUNC(caswin_state::palette_init), 0x40);

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(6'000'000) / 4)); // TODO: unconfirmed clock
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.port_b_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);
}

ROM_START( caswin )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cw_v5_0_1.26", 0x0000, 0x4000, CRC(ae3d2cf0) SHA1(268572730389f12cf962782008690305fad1ac1b) )
	ROM_LOAD( "cw_v5_0_2.24", 0x4000, 0x4000, CRC(2855b3b8) SHA1(f5cc0bbeee6c1fb0dc6aebc2e3af09dccdb248ad) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "cw_4.19", 0x00000, 0x4000, CRC(d2deab75) SHA1(12cf3fd02dbad9a40cfa6cece0cb66ce2c4dc315) )
	ROM_LOAD( "cw_3.22", 0x04000, 0x4000, CRC(7e79966c) SHA1(39190ee8cd7f3b8f895b32327f3a5555a0713315) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "clr1.bin", 0x00, 0x20, CRC(52e31046) SHA1(71a95a72b591ae7b75af4adff526fca9ae055c5b) )
	ROM_LOAD( "clr2.bin", 0x20, 0x20, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) )
ROM_END

ROM_START( rcasino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ri-w1.18b", 0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "ri-w2.16b", 0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "ri-w3.15b", 0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ri-w6.8b",  0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "ri-w7.6b",  0x2000, 0x1000, CRC(8e0d3b9c) SHA1(c5211d834b0db488839a5c53d00435a0b59cd4ca) )
	ROM_LOAD( "ri-w5.9b",  0x3000, 0x2000, CRC(81d20577) SHA1(50a1e0231400c106539ffa78deb3e0e6c8afc3f5) )
	ROM_LOAD( "ri-w4.11b", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "prom1.9e",  0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) )
	ROM_LOAD( "prom2.8e",  0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) )
ROM_END

ROM_START( rcasino1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mrdu1.f1",  0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "mrdu2.f2",  0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "mrdu3.f4",  0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "6.e11",     0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "v7.e13",    0x2000, 0x1000, CRC(c7ff4ce3) SHA1(51d4bafddcaef355571bd32b16753b1cee54368d) )
	ROM_LOAD( "h.e9",      0x3000, 0x2000, CRC(645c7cbf) SHA1(5790422b86a59531764233dd3c9488fdbad476bc) )
	ROM_LOAD( "mrdu4.e10", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "v1.a11",    0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) ) /* TBP18S030 */
	ROM_LOAD( "v2.a12",    0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) ) /* TBP18S030 */
ROM_END

ROM_START( rcasinoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mrdu1.b18", 0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "mrdu2.b16", 0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "mrdu3.b15", 0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "5.b8",      0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "6.b6",      0x2000, 0x1000, CRC(c7ff4ce3) SHA1(51d4bafddcaef355571bd32b16753b1cee54368d) )
	ROM_LOAD( "3.b9",      0x3000, 0x2000, CRC(81d20577) SHA1(50a1e0231400c106539ffa78deb3e0e6c8afc3f5) )
	ROM_LOAD( "mrdu4.b11", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "prom2.e9",  0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) ) /* MB7051 */
	ROM_LOAD( "prom1.e8",  0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) ) /* MB7051 */
ROM_END

ROM_START( rcasino2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.b18",  0x0000, 0x2000, CRC(ce04c911) SHA1(776af4b07ee57503cf48f5f0651f1e506ae4ec08) )
	ROM_LOAD( "1.b16",  0x2000, 0x2000, CRC(922b7745) SHA1(4fcf3fc02cac021820ddec89c1f57e7249ff819a) )
	ROM_LOAD( "2.b15",  0x4000, 0x1000, CRC(b4c99c55) SHA1(e90967d53c4a33cfaace603e3a9eb7e43e747101) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "5.b8",  0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "6.b6",  0x2000, 0x1000, CRC(8e0d3b9c) SHA1(c5211d834b0db488839a5c53d00435a0b59cd4ca) )
	ROM_LOAD( "4.b9",  0x3000, 0x2000, CRC(81d20577) SHA1(50a1e0231400c106539ffa78deb3e0e6c8afc3f5) )
	ROM_LOAD( "3.b11", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "mb7051.e9", 0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) )
	ROM_LOAD( "mb7051.e8", 0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) )
ROM_END

} // anonymous namespace


GAME( 1984, rcasino,  0,       vvillage, vvillage, caswin_state, empty_init, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-2)",                MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, rcasino1, rcasino, vvillage, vvillage, caswin_state, empty_init, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-1, Larger Board)",  MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, rcasinoo, rcasino, vvillage, vvillage, caswin_state, empty_init, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-1, Smaller Board, set 1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, rcasino2, rcasino, vvillage, vvillage, caswin_state, empty_init, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-1, Smaller Board, set 2)",  MACHINE_IMPERFECT_GRAPHICS )
GAME( 1985, caswin,   rcasino, vvillage, vvillage, caswin_state, empty_init, ROT270, "Aristocrat",       "Casino Winner",                               MACHINE_IMPERFECT_GRAPHICS )
