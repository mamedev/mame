// license:BSD-3-Clause
// copyright-holders: David Haywood

/***************************************************************************************************
    World Rally (Modular System).

    This is neither a clone nor a bootleg, but the original prototype for Gaelco World Rally, that
    was originally developed using the Modular System.

    This Modular System seems like a development unit, labeled PDS (Programmers Development System).

    For this game the Modular System cage contains 7 main boards and 1 small sub-board.

    MOD 5/1: (sprites) 8 x EPROM, 2 x GAL16V9
    MOD 51/1: 1 x PROM, 1 x PAL16R6, logic.
    MOD 7/3: 2 x CXK58256PM-10 RAM, 3 x GAL16V8, 2 x GAL20V8
    MOD 8/2: (gfx) 20 x EPROM.
    MOD 6-ESP/2: (CPU) MC68000P10, 20 MHz xtal, 10 x EPROM, 2 x GAL16V8.
    AUD-OKI: (aux sound, clearly a development PCB) OKI M6295, 1 MHz xtal, 1 x GAL16V8.
    SYSTEM 2: (sound) 1 x EPROM, NEC D780C-2, 28 MHz xtal, 16 MHz xtal, 1 x OKI5205, 384 kHz osc,
              1 x GAL16V8, 1xGAL20V8, 1 x 5A12 (common rebadge for Yamaha YM3014/YM3812) on a
              socket intended for a YB2151/AY2203 (and another empty YB2151/AY2203 socket).
    VOLANTE MODULAR: (small sub-board) Logic for the driving wheel.

    TODO:
    - Sound implementation may not be totally correct (although regular Modular sound seems
      completely unused)
    - Wheel support
    - Dipswitches
    - Identify unimplemented video registers
    - What is the point in the 'overlay' ROM? it contains a valid boot vector, unlike the
      main program ROMs, but otherwise doesn't work with this hardware setup.  A similar
      program exists on the glass prototype (probably to communicate with the dev hardware)
      but that had RAM instead of program ROM for the rest of the game.

***************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"

#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class wrally_ms_state : public driver_device
{
public:
	wrally_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vram(*this, "vram")
		, m_bg_vram(*this, "bg_vram")
		, m_bg2_vram(*this, "bg2_vram")
		, m_spriteram(*this, "spriteram")
		, m_scrollregs(*this, "scrollregs")
		, m_prgbank(*this,"prgbank")
		, m_okibank(*this, "okibank")
		, m_oki(*this,"oki")
	{ }

	void wrally_ms(machine_config &config) ATTR_COLD;

	void init_wrally_ms() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u16> m_vram;
	required_shared_ptr<u16> m_bg_vram;
	required_shared_ptr<u16> m_bg2_vram;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_scrollregs;
	required_memory_bank m_prgbank;
	required_memory_bank m_okibank;
	required_device<okim6295_device> m_oki;

	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_bg2_tilemap = nullptr;

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);

	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg2_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void okim6295_bankswitch_w(u8 data);

	void descramble_16x16tiles(u8 *src, int len) ATTR_COLD;

	u16 unk_r() { return machine().rand(); }
	u16 unk2_r() { return 0x0000; }

	void wrally_ms_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map);
	void oki_map(address_map &map) ATTR_COLD;
};

TILE_GET_INFO_MEMBER(wrally_ms_state::get_tile_info)
{
	u16 code = m_vram[tile_index * 2];
	u16 attr = m_vram[(tile_index * 2) + 1];
	int fx = (attr & 0xc0) >> 6;

	tileinfo.set(0, code & 0xfff, attr & 0xf, TILE_FLIPYX(fx));
}

void wrally_ms_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(wrally_ms_state::get_bg_tile_info)
{
	u16 code = m_bg_vram[tile_index * 2];
	u16 attr = m_bg_vram[(tile_index * 2) + 1];
	int fx = (attr & 0xc0) >> 6;

	tileinfo.set(2, code & 0xfff, attr & 0x1f, TILE_FLIPYX(fx));
	tileinfo.category = (attr & 0x0020) >> 5;
}

void wrally_ms_state::bg_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_vram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(wrally_ms_state::get_bg2_tile_info)
{
	u16 code = m_bg2_vram[tile_index * 2];
	u16 attr = m_bg2_vram[(tile_index * 2) + 1];
	int fx = (attr & 0xc0) >> 6;
	tileinfo.set(1, code & 0x1fff, attr & 0x1f, TILE_FLIPYX(fx));
	//tileinfo.category = (attr & 0x0020) >> 5; // not used on this layer?
}

void wrally_ms_state::bg2_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg2_vram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset/2);
}

void wrally_ms_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wrally_ms_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wrally_ms_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wrally_ms_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transparent_pen(0);
//  m_bg2_tilemap->set_transparent_pen(0);
}

void wrally_ms_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO, convert to device, share between Modular System games
	const int NUM_SPRITES = 0x200;
	const int X_EXTRA_OFFSET = 78;

	for (int i = 0; i < NUM_SPRITES; i += 2)
	{
		gfx_element* gfx = m_gfxdecode->gfx(3);

		u16 attr0 = m_spriteram[i + 0];
		u16 attr1 = m_spriteram[i + 1];

		u16 attr2 = m_spriteram[i + NUM_SPRITES];
		//u16 attr3 = m_spriteram[i + NUM_SPRITES+1]; // unused?

		int ypos = attr0 & 0x00ff;

		// unknown, unused sprites (sometimes garbage sprites) have a ypos of 00f8 and not much else
		if (ypos == 0x00f8)
			continue;

		int xpos = (attr1 & 0xff00) >> 8;

		xpos |= (attr2 & 0x8000) ? 0x100 : 0x000;

		ypos = (0xff - ypos);
		ypos |= (attr2 & 0x4000) ? 0x100 : 0x000; // maybe

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x003f) << 8;

		int flipx = (attr1 & 0x0040);
		int flipy = (attr1 & 0x0080);
		int col = (attr2 & 0x0f00) >> 8;

		int pri_mask;
		if (col & 8)
			pri_mask = 0xf0;
		else
			pri_mask = 0x00;

		gfx->prio_transpen(bitmap, cliprect, tile, col, flipx, flipy, xpos - 16 - X_EXTRA_OFFSET, ypos - 16, screen.priority(), pri_mask, 15);
	}
}

u32 wrally_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	// are these using the correct reg pairs?
	m_bg_tilemap->set_scrollx(0, 80-(m_scrollregs[0]));
	m_bg_tilemap->set_scrolly(0, -m_scrollregs[1]);

	m_bg2_tilemap->set_scrollx(0, 80-(m_scrollregs[6]));
	m_bg2_tilemap->set_scrolly(0, -m_scrollregs[7]);

	// what are m_scrollregs 2,3 and 4,5?
	// one is probably tx scroll, the other priority control (or not?)

	m_tx_tilemap->set_scrollx(0, 80);
	m_tx_tilemap->set_scrolly(0, 0);

	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 2);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 4);

	draw_sprites(screen, bitmap, cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void wrally_ms_state::wrally_ms_map(address_map &map)
{
	map(0x000000, 0x01ffff).bankr("prgbank");
	map(0x020000, 0x0fffff).rom().region("maincpu", 0x20000).nopw(); // sometimes writes to the ROM area (code bug?)

	map(0x100000, 0x1007ff).ram().share("spriteram");

	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x300000, 0x301fff).ram().w(FUNC(wrally_ms_state::vram_w)).share("vram");
	map(0x340000, 0x341fff).ram().w(FUNC(wrally_ms_state::bg_w)).share("bg_vram");
	map(0x380000, 0x381fff).ram().w(FUNC(wrally_ms_state::bg2_w)).share("bg2_vram");
	// sometimes writes to 382000, 382002 (code bug?)

	map(0x3c0000, 0x3c000f).ram().share("scrollregs"); // video regs

	map(0x400000, 0x400001).portr("IN0");
	map(0x400002, 0x400003).portr("IN1");
	map(0x400004, 0x400005).r(FUNC(wrally_ms_state::unk_r));
	map(0x400006, 0x400007).portr("DSW1");
	map(0x400008, 0x400009).portr("DSW2");
	map(0x40000c, 0x40000d).r(FUNC(wrally_ms_state::unk_r)).nopw(); // writes 00 sometimes

//  map(0x600000, 0x600001).r(FUNC(wrally_ms_state::unk2_r));
	map(0x600081, 0x600081).mirror(0x100).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x600101, 0x600101).w(FUNC(wrally_ms_state::okim6295_bankswitch_w));
	map(0xff0000, 0xffffff).ram();
}

void wrally_ms_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe800, 0xe801).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xf000, 0xffff).ram();
}

void wrally_ms_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( wrally_ms )
	// some modular games read inputs in the high byte, others low, maybe it mirrors?
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0800, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Control Type" ) PORT_DIPLOCATION("SW2:6") // currently locks up after a few seconds of gameplay with wheel
	PORT_DIPSETTING(      0x2000, "Wheel" )
	PORT_DIPSETTING(      0x0000, "Joystick" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,32) },
	16 * 16
};

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static GFXDECODE_START( gfx_wrally_ms )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16x4_layout, 0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16x4_layout, 0, 32 )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x200, 16 )
GFXDECODE_END

void wrally_ms_state::machine_start()
{
	m_prgbank->configure_entry(0, memregion("maincpu")->base());
	m_prgbank->configure_entry(1, memregion("maincpu")->base() + 0x100000);
	m_prgbank->set_entry(0);
	//m_prgbank->set_entry(1); // use overlay ROM instead (doesn't upload a valid palette?)

	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

void wrally_ms_state::okim6295_bankswitch_w(u8 data)
{
	m_okibank->set_entry(data & 0x0f);
}


// Reorganize graphics into something we can decode with a single pass
void wrally_ms_state::descramble_16x16tiles(u8 *src, int len)
{
	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19,18,17,16,15,12,11,10,9,8,7,6,5,14,13,4,3,2,1,0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void wrally_ms_state::init_wrally_ms()
{
	descramble_16x16tiles(memregion("gfx2")->base(), memregion("gfx2")->bytes());
	descramble_16x16tiles(memregion("gfx3")->base(), memregion("gfx3")->bytes());
}

void wrally_ms_state::wrally_ms(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wrally_ms_state::wrally_ms_map);
	m_maincpu->set_vblank_int("screen", FUNC(wrally_ms_state::irq4_line_hold));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 46*8-1, 0*8, 29*8-1); // visarea based on test mode
	m_screen->set_screen_update(FUNC(wrally_ms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 0x400);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_wrally_ms);

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_HIGH); // PIN 7 seems like it's not connected to anything at all
	m_oki->set_addrmap(0, &wrally_ms_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.7);

	// the sound hardware below seems to go unused, the program is the same as Splash (Modular System)
	// all sounds for this are from the OKIM6295, driven by the 68000

	Z80(config, m_soundcpu, 16_MHz_XTAL / 4); // NEC D780C-2
	m_soundcpu->set_addrmap(AS_PROGRAM, &wrally_ms_state::sound_map);

	YM3812(config, "ymsnd", 16_MHz_XTAL / 4); // Unknown divisor

	MSM5205(config, "msm", 384_kHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.15);
}

ROM_START( wrallymp )
	ROM_REGION( 0x120000, "maincpu", 0 )
	// These were a row of 8 ROMs next to the M68000, they appear to form the start of the code, but the initial boot vector is wrong
	ROM_LOAD16_BYTE( "mod_6-esp-2_z0_e_25-11_27c1001.u8", 0x000000, 0x020000, CRC(a0d200eb) SHA1(fbca9b84d8b010aa0bfb546ddf366ec9812f0bb5) )
	ROM_LOAD16_BYTE( "mod_6-esp-2_z0_o_25-11_27c1001.u7", 0x000001, 0x020000, CRC(4113a030) SHA1(e6cac2b0e97ec7f15610aa72198a575631b937f6) )
	ROM_FILL( 0x06, 1, 0x24 ) // fix the boot vector as we're using this set of program ROMs

	// FIXED BITS (0xxxxxxx) on z1_e - but intentional? 15-bit data tables?
	ROM_LOAD16_BYTE( "mod_6-esp-2_z1_e_25-11_27c1001.u36", 0x040000, 0x020000, CRC(4346b650) SHA1(a0feca5d9d93af2548b59c11032384703d432a30) )
	ROM_LOAD16_BYTE( "mod_6-esp-2_z1_o_25-11_27c1001.u33", 0x040001, 0x020000, CRC(9139cbcb) SHA1(93a3d4a2cc08f56517ca74aa6ed5bbb08821ec17) )

	// FIXED BITS (0xxxxxxx) on z2_e - but intentional? 15-bit data tables?
	ROM_LOAD16_BYTE( "mod_6-esp-2_z2_e_25-11_27c1001.u37", 0x080000, 0x020000, CRC(a4e3fa03) SHA1(065429304d1449b9fb8db2708c86b7371e7c3362) )
	ROM_LOAD16_BYTE( "mod_6-esp-2_z2_o_25-11_27c1001.u34", 0x080001, 0x020000, CRC(fdfd11e9) SHA1(43ca655caef745ec2c64dd85a0ebab73b51cb654) )

	ROM_LOAD16_BYTE( "mod_6-esp-2_z3_e_25-11_27c1001.u38", 0x0c0000, 0x020000, CRC(b274f9b1) SHA1(bf47c903e8f535d2cbcbbeb8592c5d68ca2d7e40) )
	ROM_LOAD16_BYTE( "mod_6-esp-2_z3_o_25-11_27c1001.u35", 0x0c0001, 0x020000, CRC(8ea3bad5) SHA1(7470ff853ba0c419adf1164416c9e62ef78a50b2) )

	// Does this act as an overlay? It's basically the same as the above, but with a valid boot vector and some other changes.
	// Was found on the PCB just above the 8 loaded previously
	// running these however just results in bad palette uploads for test mode and a crash outside of that
	ROM_LOAD16_BYTE( "mod_6-esp-2_pds_0_coche_27c512.u6", 0x100000, 0x010000, CRC(362c8f1e) SHA1(013499bf78fc9806f988354ca17e99e9cc2f7f71) )
	ROM_LOAD16_BYTE( "mod_6-esp-2_pds_1_coche_27c512.u5", 0x100001, 0x010000, CRC(f87e0e9b) SHA1(55eec7612baede958e0abffe426945d85726ffdc) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // 8x8
	// all ROMs 11xxxxxxxxxxxxxx = 0x00 (last 3/4 0x00)
	ROM_LOAD32_BYTE( "mod_8-2_fijas_a_23-11_27c512.ic15", 0x000003, 0x4000, CRC(0241312b) SHA1(3a912731cd85bfdf4789d994580729f407eaace2) )
	ROM_IGNORE(0xc000)
	ROM_LOAD32_BYTE( "mod_8-2_fijas_b_23-11_27c512.ic22", 0x000002, 0x4000, CRC(44a2fc73) SHA1(90353c54ec0bd26f3e290b3b03cbfc120b85cfe2) )
	ROM_IGNORE(0xc000)
	ROM_LOAD32_BYTE( "mod_8-2_fijas_c_23-11_27c512.ic30", 0x000001, 0x4000, CRC(b9e94ba5) SHA1(50c9ba5455a7383f07f13a851d50444477e83ff4) )
	ROM_IGNORE(0xc000)
	ROM_LOAD32_BYTE( "mod_8-2_fijas_d_23-11_27c512.ic37", 0x000000, 0x4000, CRC(96644b11) SHA1(4f6a972610ad043d13df6157975739be141ff1e7) )
	ROM_IGNORE(0xc000)

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_la-i_27c010.ic13", 0x000003, 0x10000, CRC(c1b2f2e6) SHA1(4555f0024289d395484c172cff58544b73e92ddb) )
	ROM_CONTINUE(0x080003, 0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_lb-j_27c010.ic20", 0x000002, 0x10000, CRC(f0dbd657) SHA1(2fb269d3e90d1211f1c4ecf4ef4c785fed0f1111) )
	ROM_CONTINUE(0x080002, 0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_lc_k_27c010.ic28", 0x000001, 0x10000, CRC(f09f8067) SHA1(bbfe21cefce2307f87a3416af1767fa4b30da446) )
	ROM_CONTINUE(0x080001, 0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ld-l_27c010.ic35", 0x000000, 0x10000, CRC(4feb7aab) SHA1(86f222cb95bd7366ec921b2da438847c3679ab46) )
	ROM_CONTINUE(0x080000, 0x10000)
	// All ROMs 1xxxxxxxxxxxxxxxx = 0xFF (2nd half 0xff)
	ROM_LOAD32_BYTE( "mod_8-2_fon_he_27c1001.ic12", 0x040003, 0x10000, CRC(0279bb03) SHA1(fde2613164651c738469bbfe1a5918c89c0f3cb2) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_fon_hf_27c1001.ic19", 0x040002, 0x10000, CRC(221c3249) SHA1(2a8f3d93dddc38ed38ee330687fc970507316b02) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_fon_hg_27c1001.ic27", 0x040001, 0x10000, CRC(f186dcca) SHA1(6dbf2438862592bb0e8647b6aaca4747ad8e2755) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_fon_hh_27c1001.ic34", 0x040000, 0x10000, CRC(2e73266c) SHA1(1a7a668483e30bf664aa037f86d4a73033dab83b) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "gfx3", 0 )
	// All ROMs 1xxxxxxxxxxxxxxxx = 0x00 (2nd half 0x00)
	ROM_LOAD32_BYTE( "mod_8-2_ned_la_27c1001.ic11", 0x000003, 0x10000, CRC(6b8b4d0d) SHA1(165a6e54004fd2249a8a26554a45a6940ca3873f) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ned_lb_27c1001.ic18", 0x000002, 0x10000, CRC(f00783aa) SHA1(3f2025dba2dfa863b754aaaadf0e0de4a2546c0c) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ned_lc_27c1001.ic26", 0x000001, 0x10000, CRC(f346ad2c) SHA1(2e335dab1d6a7cc7087e7be17013e1bd63f22f51) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ned_ld_27c1001.ic33", 0x000000, 0x10000, CRC(79e991af) SHA1(45163d8692926512e47ddfea8c32848a74910e9d) )
	ROM_IGNORE(0x10000)
	// All ROMs 1xxxxxxxxxxxxxxxx = 0x00 (2nd half 0x00)
	ROM_LOAD32_BYTE( "mod_8-2_hed_he_27c1001.ic9",  0x040003, 0x10000, CRC(9f90fa11) SHA1(f5ebebb8e9cab802193ebd61449ca922ed9db380) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ned_hf_27c1001.ic17", 0x040002, 0x10000, CRC(b3ae8c46) SHA1(f37bbd8ac5a3e0b3a3419ef9cf7bea5fe9600c77) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ned_hg_27c1001.ic25", 0x040001, 0x10000, CRC(d362172d) SHA1(f9b1e26a8b4b55e48e5b94ba9270e2e3bf82fec2) )
	ROM_IGNORE(0x10000)
	ROM_LOAD32_BYTE( "mod_8-2_ned_hh_27c1001.ic32", 0x040000, 0x10000, CRC(4a3e115c) SHA1(130094a6a4bb62f61a42e9e9cef12ae7215724f7) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_5-1_mov_la_23-11_27c512.ic3",  0x000003, 0x010000, CRC(8a5f3713) SHA1(73a8ccebfe55daf17ab91cc57cdca866477ba09f) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_lb_23-11_27c512.ic12", 0x000002, 0x010000, CRC(a410e537) SHA1(f15a8f9f3951a428decd7ebf64881e8dcd959060) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_lc_23-11_27c512.ic18", 0x000001, 0x010000, CRC(94551ca4) SHA1(bc72f092be9646b89f2c7d304a7e8b87d91b9f31) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_ld_23-11_27c512.ic24", 0x000000, 0x010000, CRC(7db6e321) SHA1(19a81c122c48926f377a844f3eb2d17613e8be03) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_ha_23-11_27c512.ic4",  0x040003, 0x010000, CRC(db0e9342) SHA1(c80fdf1653cb9218d8db25daf14983d8b73f5225) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_hb_23-11_27c512.ic13", 0x040002, 0x010000, CRC(8df327ec) SHA1(f66014dd593464230cff94440abb4c63051121e1) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_hc_23-11_27c512.ic19", 0x040001, 0x010000, CRC(32733ee6) SHA1(abcc60b4cb6a672f5e68c16fa634da839d4dc38a) )
	ROM_LOAD32_BYTE( "mod_5-1_mov_hd_23-11_27c512.ic25", 0x040000, 0x010000, CRC(fc7ddb14) SHA1(80b00269498d704775fd85518fc451f00acadc0a) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "mod_9-2_sp_906_27c512.u6", 0x000000, 0x010000, CRC(5567fa22) SHA1(3993c733a0222ca292b60f409c78b45280a5fec6) ) // same as Splash (Modular System)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "aud-oki_sorally_1_27c040.bin", 0x000000, 0x080000, CRC(449aae4a) SHA1(3915e7b85fde0521c89c206c0022db6477f2797e) )
	ROM_LOAD( "aud-oki_sorally_2_27c040.bin", 0x080000, 0x080000, CRC(09967f05) SHA1(56306414bcc73e0880d3f069666f3c696d687009) )

	ROM_REGION( 0x100, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_51-1_502_82s129a.ic10", 0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular system

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_51-1_p0503_pal16r6a.ic46",   0x0000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	ROM_LOAD( "mod_7-3_71_espe_gal20v8as.ic7",  0x0000, 0x157, CRC(b0644601) SHA1(8b0410006bdffe552ee27d027bdb5b3c642fe49c) )
	ROM_LOAD( "mod_7-3_72fl_gal20v8as.ic54",    0x0000, 0x157, CRC(e9ec1e9e) SHA1(90d74aa6e15a3bba93c8264baffc72545be1474f) )
	ROM_LOAD( "mod_7-3_73fl_gal16v8as.ic55",    0x0000, 0x117, CRC(e112efd9) SHA1(8005da091200f2482c0ff5bf8f9c0fd2ab767fac) )
	ROM_LOAD( "mod_7-3_74fl_gal16v8as.ic9",     0x0000, 0x117, CRC(2634cce1) SHA1(e7ac65e24bae2973eb1ac914797e3b3140c267eb) )
	ROM_LOAD( "mod_7-3_75fl_gal16v8as.ic59",    0x0000, 0x117, CRC(b3e2f5c5) SHA1(d03fbcb28818c8a29ed42369ce4a6734f2a87b4f) )
	ROM_LOAD( "mod_7-3_76fl_gal20v8as.ic44",    0x0000, 0x157, CRC(c1dba1d1) SHA1(cc12fe58b3d597e8c5aa3b3d8c5b41ba1ea3ad32) )
	ROM_LOAD( "mod_5-1_52fx_gal16v8.ic8",       0x0000, 0x117, CRC(f3d686c9) SHA1(6bc1bd40f49663e776c0d40b2f146338a2057097) )
	ROM_LOAD( "mod_5-1_51fx_gal16v8.ic9",       0x0000, 0x117, CRC(0070e8b9) SHA1(8a3ef9445599dc88c001a919e01f47844eec81eb) )
	ROM_LOAD( "mod_6-esp-2_16fl_gal16v8as.u20", 0x0000, 0x117, CRC(fe9294d1) SHA1(e64539a5e661acc21fe040a1a478a63ea9bf7ab8) )
	ROM_LOAD( "mod_9-2_9350_gal16v8as.u10",     0x0000, 0x117, CRC(54b4160f) SHA1(0156a2eda97a9d8e0adb3a3795f6ed547c6e06fc) )
	ROM_LOAD( "mod_9-2_91fl_gal16v8as.u42",     0x0000, 0x117, CRC(ac8a5e11) SHA1(cbe68b38e0bce978722753a3feeafad9dcf05d25) )
	ROM_LOAD( "aud_oki_gal16v8as.bin",          0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_9-2_92fl_gal20v8as.ic18",    0x0000, 0x157, CRC(3a1465c2) SHA1(c98227c29301fdff7ab8144222ff0c257412dd78) )
ROM_END

} // anonymous namespace

// World Rally was originally developed using the Modular System, so this isn't a bootleg, it's the original development version
GAME( 1992, wrallymp, wrally, wrally_ms, wrally_ms, wrally_ms_state, init_wrally_ms, ROT0, "Gaelco", "World Rallye Championship (prototype on Modular System, 23 Nov 1992)", 0 ) // 23/11/1992
