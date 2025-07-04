// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************************
    Xor World (Modular System).

    This is neither a clone nor a bootleg, but the original Xor World, that was originally developed
    using the Modular System.

    For this game the Modular System cage contains 6 boards.

    MOD 5/1: (sprites) 4 x EPROM, 2 x GAL16V9
    MOD 51/1: 1 x PROM, 1 x PAL16R6, logic.
    MOD 7/4: 2 x MS62256L-10PC RAM, 3 x GAL16V8, 2 x GAL20V8
    MOD 8/2: (gfx) 12 x EPROM.
    MOD 6/1: (CPU) MC68000P10, 20 MHz xtal, 2 x EPROM, 1 x GAL16V8, 1 x PAL16V8.
    SYSTEM 2: (sound) 1 x EPROM, Z8400BB1, 28 MHz xtal, 16 MHz xtal, 1 x OKI5205, 384 kHz osc,
              1 x GAL16V8, 1 x GAL20V8, 1 x YM3812.

    quite similar to splash_ms.cpp but without the extra bitmap layer and CPU driving it

    TODO:
     - service mode colours are broken (probably due to being a prototype?)
     - do the higher attribute bits in the 16x16 layer have meaning? (probably not)
     - what determines sound NMI frequency on these Modular boards as it's different to splash?

***************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"

#include "machine/gen_latch.h"

#include "sound/msm5205.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class xorworld_ms_state : public driver_device
{
public:
	xorworld_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_videoram_8x8_mg(*this, "videoram_8x8mg")
		, m_videoram_8x8_mg_8x8_fg(*this, "videoram_8x8fg")
		, m_videoram_16x16_bg(*this, "videoram16x16bg")
		, m_scrollregs(*this, "scrollregs")
		, m_spriteram(*this, "spriteram")
		, m_soundlatch(*this, "soundlatch")
		, m_msm(*this, "msm")
		, m_ymsnd(*this, "ymsnd")
	{ }

	void xorworld_ms(machine_config &config) ATTR_COLD;

	void init_xorworld_ms() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_videoram_8x8_mg;
	required_shared_ptr<uint16_t> m_videoram_8x8_mg_8x8_fg;
	required_shared_ptr<uint16_t> m_videoram_16x16_bg;
	required_shared_ptr<uint16_t> m_scrollregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<msm5205_device> m_msm;
	required_device<ym3812_device> m_ymsnd;

	void descramble_16x16tiles(uint8_t* src, int len);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void videoram8x8_mg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void videoram8x8_fg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void videoram16x16_bg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap_8x8mg);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap_8x8fg);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap_16x16bg);

	void xorworld_ms_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	void msm5205_int(int state);
	void adpcm_data_w(uint8_t data);
	void adpcm_control_w(uint8_t data);
	int m_adpcm_data = 0;

	tilemap_t *m_tilemap_8x8_mg = nullptr;
	tilemap_t *m_tilemap_8x8_fg = nullptr;
	tilemap_t *m_tilemap_16x16_bg = nullptr;
};


void xorworld_ms_state::video_start()
{
	m_tilemap_8x8_mg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xorworld_ms_state::get_tile_info_tilemap_8x8mg)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_tilemap_8x8_fg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xorworld_ms_state::get_tile_info_tilemap_8x8fg)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_tilemap_16x16_bg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xorworld_ms_state::get_tile_info_tilemap_16x16bg)), TILEMAP_SCAN_ROWS,  16,  16, 64, 32);

	m_tilemap_8x8_mg->set_transparent_pen(0);
	m_tilemap_8x8_fg->set_transparent_pen(0);
}

u32 xorworld_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// based on other Modular games I think this is correct
	// scrollregs 0/1 for layer at 90000 (middle layer?)
	// scrollregs 2/3 for sprite global offsets?
	// scrollregs 4/5 for layer at 80000 (frontmost?)
	// scrollregs 6/7 for layer at a0000 (backmost)
	int global_x_offset = 128;

	m_tilemap_8x8_fg->set_scrollx(0, global_x_offset-(m_scrollregs[4]));
	m_tilemap_8x8_fg->set_scrolly(0, -(m_scrollregs[5]));
	m_tilemap_8x8_mg->set_scrollx(0, global_x_offset-(m_scrollregs[0]));
	m_tilemap_8x8_mg->set_scrolly(0, -(m_scrollregs[1]));
	m_tilemap_16x16_bg->set_scrollx(0, global_x_offset-(m_scrollregs[6]));
	m_tilemap_16x16_bg->set_scrolly(0, -(m_scrollregs[7])); // ugly, but based on other Modular games, probably correct!

	m_tilemap_16x16_bg->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap_8x8_mg->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap_8x8_fg->draw(screen, bitmap, cliprect, 0, 0);

	int spritexoffset = m_scrollregs[2] & 0xfff;
	if (spritexoffset & 0x800)
		spritexoffset -= 0x1000;

	// TODO, convert to device, share between Modular System games
	const int NUM_SPRITES = 0x200;
	const int X_EXTRA_OFFSET = global_x_offset + spritexoffset;
	// m_scrollregs[3] would be sprite y offset?

	for (int i = NUM_SPRITES - 2; i >= 0; i -= 2)
	{
		gfx_element* gfx = m_gfxdecode->gfx(0);

		uint16_t attr0 = m_spriteram[i + 0];
		uint16_t attr1 = m_spriteram[i + 1];

		uint16_t attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t attr3 = m_spriteram[i + NUM_SPRITES+1]; // unused?

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

		gfx->transpen(bitmap, cliprect, tile, (attr2 & 0x0f00) >> 8, flipx, flipy, xpos - 16 - X_EXTRA_OFFSET, ypos - 16, 15);
	}

	return 0;
}

void xorworld_ms_state::videoram8x8_mg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram_8x8_mg[offset]);
	m_tilemap_8x8_mg->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(xorworld_ms_state::get_tile_info_tilemap_8x8mg)
{
	int tile = m_videoram_8x8_mg[tile_index * 2];
	int attr = m_videoram_8x8_mg[(tile_index * 2) + 1] & 0x1f;
	//  int fx = (m_videoram_8x8_mg[(tile_index*2)+1] & 0xc0)>>6;

	tileinfo.set(2, tile, attr, 0); // must be region 2 for the score display tiles
}

void xorworld_ms_state::videoram8x8_fg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram_8x8_mg_8x8_fg[offset]);
	m_tilemap_8x8_fg->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(xorworld_ms_state::get_tile_info_tilemap_8x8fg)
{
	int tile = m_videoram_8x8_mg_8x8_fg[tile_index * 2];
	int attr = m_videoram_8x8_mg_8x8_fg[(tile_index * 2) + 1] & 0x1f;
	//  int fx = (m_videoram_8x8_mg[(tile_index*2)+1] & 0xc0)>>6;

	tileinfo.set(1, tile, attr, 0); // assume region 1 as other tilemap is region 2
}

void xorworld_ms_state::videoram16x16_bg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram_16x16_bg[offset]);
	m_tilemap_16x16_bg->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(xorworld_ms_state::get_tile_info_tilemap_16x16bg)
{
	// some other upper bits in attributes are used, but on areas
	// that are intentionally invisible?
	int tile = m_videoram_16x16_bg[tile_index * 2];
	int attr = (m_videoram_16x16_bg[(tile_index * 2) + 1]);

	tileinfo.set(3, tile, attr & 0x1f, 0);
}

void xorworld_ms_state::xorworld_ms_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x080000, 0x081fff).ram().w(FUNC(xorworld_ms_state::videoram8x8_fg_w)).share("videoram_8x8fg");
	map(0x090000, 0x091fff).ram().w(FUNC(xorworld_ms_state::videoram8x8_mg_w)).share("videoram_8x8mg");
	map(0x0a0000, 0x0a1fff).ram().w(FUNC(xorworld_ms_state::videoram16x16_bg_w)).share("videoram16x16bg");

	map(0x0c0000, 0x0c000f).ram().share("scrollregs");

	map(0x100000, 0x1007ff).ram().share("spriteram");
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x400000, 0x400001).portr("IN0");
	map(0x400002, 0x400003).portr("IN1");
	map(0x400006, 0x400007).portr("DSW1");
	map(0x400008, 0x400009).portr("DSW2");
	map(0x40000c, 0x40000d).noprw(); // unknown
	map(0x40000e, 0x40000e).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0xff0000, 0xffffff).ram();
}

void xorworld_ms_state::adpcm_data_w(uint8_t data)
{
	m_adpcm_data = data;
}

void xorworld_ms_state::adpcm_control_w(uint8_t data)
{
	m_msm->reset_w(BIT(data, 7));
}

void xorworld_ms_state::msm5205_int(int state)
{
	m_msm->data_w(m_adpcm_data >> 4);
	m_adpcm_data = (m_adpcm_data << 4) & 0xf0;
}

void xorworld_ms_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xe000).w(FUNC(xorworld_ms_state::adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(xorworld_ms_state::adpcm_data_w));

	map(0xe800, 0xe801).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));

	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( xorworld_ms )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Button 3 / P1 Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Button 3 / P2 Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) // unused according to service mode
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	// no 2nd dip bank?
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
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

static GFXDECODE_START( gfx_xorworld_ms )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx_8x8fg", 0, tiles8x8x4_layout, 0x000, 32 )
	GFXDECODE_ENTRY( "gfx_8x8mg", 0, tiles8x8x4_layout, 0x000, 32 )
	GFXDECODE_ENTRY( "gfx_16x16bg", 0, tiles16x16x4_layout, 0x000, 32 )
GFXDECODE_END

// reorganize graphics into something we can decode with a single pass
void xorworld_ms_state::descramble_16x16tiles(u8* src, int len)
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

void xorworld_ms_state::machine_start()
{
	save_item(NAME(m_adpcm_data));
}

void xorworld_ms_state::init_xorworld_ms()
{
	descramble_16x16tiles(memregion("gfx_16x16bg")->base(), memregion("gfx_16x16bg")->bytes());
}

void xorworld_ms_state::xorworld_ms(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &xorworld_ms_state::xorworld_ms_map);
	m_maincpu->set_vblank_int("screen", FUNC(xorworld_ms_state::irq4_line_hold));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(xorworld_ms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 0x400);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_xorworld_ms);

	// Sound hardware
	Z80(config, m_soundcpu, 16_MHz_XTAL/4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &xorworld_ms_state::sound_map);
	m_soundcpu->set_periodic_int(FUNC(xorworld_ms_state::nmi_line_pulse), attotime::from_hz(2500));

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);

	SPEAKER(config, "mono").front_center();

	YM3812(config, m_ymsnd, XTAL(16'000'000) / 4);
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 0.80);
	// YM3812 does not set the IRQ line, so can't be driving tempo
	//m_ymsnd->irq_handler().set(FUNC(xorworld_ms_state::ym_irq));

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(xorworld_ms_state::msm5205_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}

ROM_START( xorwldms )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mod_6-1_xo_608a_27c512.ic8",  0x000001, 0x010000, CRC(cebcd3e7) SHA1(ee6d107dd13e4faa8d5b6ba9815c57919a69568e) )
	ROM_LOAD16_BYTE( "mod_6-1_xo_617a_27c512.ic17", 0x000000, 0x010000, CRC(47bae292) SHA1(f36e2f80d4da31d7edc7dc8c07abb158ef21cb20) )

	ROM_REGION( 0x40000, "gfx_8x8fg", ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic15", 0x00003, 0x10000, CRC(01d16cac) SHA1(645b3380e66ab158392f8161485ae957ab2dfa44) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic22", 0x00002, 0x10000, CRC(3aadacaf) SHA1(f640a1d6a32774cb70e180c49965f70f5b79ba6a) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic30", 0x00001, 0x10000, CRC(fa75826e) SHA1(1db9cc8ec5811b9d497b060bfc3245c6e082e98c) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic37", 0x00000, 0x10000, CRC(157832ed) SHA1(5dec1c7046d2449d3d4330654554f1a9430c8057) )

	ROM_REGION( 0x40000, "gfx_8x8mg", 0 ) // *almost* the same as gfx1 but not inverted, some tiles changed
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic11", 0x00003, 0x10000, CRC(93373f1f) SHA1(06718a09c76060914a1b2a4c57d0328a9f59eb99) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic18", 0x00002, 0x10000, CRC(7015d8ff) SHA1(83d40ef2ca1cf8dfb2229b8f47c08a303b8490ea) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic26", 0x00001, 0x10000, CRC(4be8db92) SHA1(8983893570587de9cdd63bc645d1ae2d8c3400a2) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic33", 0x00000, 0x10000, CRC(c29cd83b) SHA1(e8833b6ab8f7f6ce4c01937e94332a86908353db) )

	ROM_REGION( 0x40000, "gfx_16x16bg", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic13", 0x00003, 0x10000, CRC(56a683fc) SHA1(986a6b26e38308dd3230dd13a1a2f5cfc7b1dda8) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic20", 0x00002, 0x10000, CRC(950090e6) SHA1(cdec2eb93884c5a759af39d02f5cc0fa25011103) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic28", 0x00001, 0x10000, CRC(ca950a11) SHA1(cef2a65d1f5562556fc28a95f6ba8f3ff1a3678d) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic35", 0x00000, 0x10000, CRC(5e7582f9) SHA1(fdb3ddb2a224fdea610f6bd55a3e3efe1a5515a2) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic3",  0x00003, 0x10000, CRC(26b2181e) SHA1(03cc280a4e9d9d8ac1c04da7d90c684d83fc2444) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic12", 0x00002, 0x10000, CRC(b2bdb8d1) SHA1(eeace8dc4e6c192ad7b2f53cd01fcc2e92125f35) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic18", 0x00001, 0x10000, CRC(d0c7a07b) SHA1(698883f2a91a34c802c1bc9a86bec0a77f4de7fe) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic24", 0x00000, 0x10000, CRC(fd2cbaed) SHA1(037523a3eae058e848763bbfc067d8271c70074b) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "mod_9-2_27c512.ic6", 0x00000, 0x10000, CRC(c722165b) SHA1(b1cd3ac80521036059579e71b6cb9bd4b97da4c2) )

	ROM_REGION( 0x100, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_51-3_502_82s129a.ic10", 0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular system

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_5-1_52fx_gal16v8.ic8",  0x0000, 0x117, CRC(f3d686c9) SHA1(6bc1bd40f49663e776c0d40b2f146338a2057097) )
	ROM_LOAD( "mod_5-1_51fx_gal16v8.ic9",  0x0000, 0x117, CRC(0070e8b9) SHA1(8a3ef9445599dc88c001a919e01f47844eec81eb) )
	ROM_LOAD( "mod_6-1_gal16v8.ic7",       0x0000, 0x117, BAD_DUMP CRC(470a0194) SHA1(810c51fca7d2430b7c2292f3e1dd02c86d74d64b) ) // Bruteforced
	ROM_LOAD( "mod_6-1_palce16v8.ic13",    0x0000, 0x117, BAD_DUMP CRC(0b90f141) SHA1(4b0f2b98073d52b6a49941031e8e37b5aeb6d507) ) // Bruteforced
	ROM_LOAD( "mod_7-4_gal20v8.ic7",       0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic9",       0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic44",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic54",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic55",      0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic59",      0x0000, 0x117, BAD_DUMP CRC(55b86dd5) SHA1(4a05d86002f929f38171282d2aad5d986283c45b) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal16v8.ic10",      0x0000, 0x117, BAD_DUMP CRC(23070765) SHA1(3c87f7ab299fcd62da366fc9c1985ab21ee14960) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal20v8.ic18",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_9-2_gal16v8.ic42",      0x0000, 0x117, BAD_DUMP CRC(d62f0a0d) SHA1(76bd995c8b71b12b752ed517d87a78e55a8bab00) ) // Bruteforced
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46", 0x0000, 0x117, NO_DUMP )
ROM_END

ROM_START( xorwldmsa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xo_608_27c512.bin", 0x000001, 0x010000, CRC(12ed0ab0) SHA1(dcdd6dccc367fe1084af71001afaa26c4879817e) )
	ROM_LOAD16_BYTE( "xo_617_27c512.bin", 0x000000, 0x010000, CRC(fea2750f) SHA1(b6ed781514a9c1901372e489ba46a36120bde528) )

	ROM_REGION( 0x40000, "gfx_8x8fg", ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic15", 0x00003, 0x10000, CRC(01d16cac) SHA1(645b3380e66ab158392f8161485ae957ab2dfa44) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic22", 0x00002, 0x10000, CRC(3aadacaf) SHA1(f640a1d6a32774cb70e180c49965f70f5b79ba6a) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic30", 0x00001, 0x10000, CRC(fa75826e) SHA1(1db9cc8ec5811b9d497b060bfc3245c6e082e98c) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic37", 0x00000, 0x10000, CRC(157832ed) SHA1(5dec1c7046d2449d3d4330654554f1a9430c8057) )

	ROM_REGION( 0x40000, "gfx_8x8mg", 0 ) // *almost* the same as gfx1 but not inverted, some tiles changed
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic11", 0x00003, 0x10000, CRC(93373f1f) SHA1(06718a09c76060914a1b2a4c57d0328a9f59eb99) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic18", 0x00002, 0x10000, CRC(7015d8ff) SHA1(83d40ef2ca1cf8dfb2229b8f47c08a303b8490ea) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic26", 0x00001, 0x10000, CRC(4be8db92) SHA1(8983893570587de9cdd63bc645d1ae2d8c3400a2) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic33", 0x00000, 0x10000, CRC(c29cd83b) SHA1(e8833b6ab8f7f6ce4c01937e94332a86908353db) )

	ROM_REGION( 0x40000, "gfx_16x16bg", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic13", 0x00003, 0x10000, CRC(56a683fc) SHA1(986a6b26e38308dd3230dd13a1a2f5cfc7b1dda8) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic20", 0x00002, 0x10000, CRC(950090e6) SHA1(cdec2eb93884c5a759af39d02f5cc0fa25011103) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic28", 0x00001, 0x10000, CRC(ca950a11) SHA1(cef2a65d1f5562556fc28a95f6ba8f3ff1a3678d) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic35", 0x00000, 0x10000, CRC(5e7582f9) SHA1(fdb3ddb2a224fdea610f6bd55a3e3efe1a5515a2) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic3",  0x00003, 0x10000, CRC(26b2181e) SHA1(03cc280a4e9d9d8ac1c04da7d90c684d83fc2444) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic12", 0x00002, 0x10000, CRC(b2bdb8d1) SHA1(eeace8dc4e6c192ad7b2f53cd01fcc2e92125f35) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic18", 0x00001, 0x10000, CRC(d0c7a07b) SHA1(698883f2a91a34c802c1bc9a86bec0a77f4de7fe) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic24", 0x00000, 0x10000, CRC(fd2cbaed) SHA1(037523a3eae058e848763bbfc067d8271c70074b) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "mod_9-2_27c512.ic6", 0x00000, 0x10000, CRC(c722165b) SHA1(b1cd3ac80521036059579e71b6cb9bd4b97da4c2) )

	ROM_REGION( 0x100, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_51-3_502_82s129a.ic10", 0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular system

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_5-1_52fx_gal16v8.ic8",  0x0000, 0x117, CRC(f3d686c9) SHA1(6bc1bd40f49663e776c0d40b2f146338a2057097) )
	ROM_LOAD( "mod_5-1_51fx_gal16v8.ic9",  0x0000, 0x117, CRC(0070e8b9) SHA1(8a3ef9445599dc88c001a919e01f47844eec81eb) )
	ROM_LOAD( "mod_6-1_gal16v8.ic7",       0x0000, 0x117, BAD_DUMP CRC(470a0194) SHA1(810c51fca7d2430b7c2292f3e1dd02c86d74d64b) ) // Bruteforced
	ROM_LOAD( "mod_6-1_palce16v8.ic13",    0x0000, 0x117, BAD_DUMP CRC(0b90f141) SHA1(4b0f2b98073d52b6a49941031e8e37b5aeb6d507) ) // Bruteforced
	ROM_LOAD( "mod_7-4_gal20v8.ic7",       0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic9",       0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic44",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic54",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic55",      0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic59",      0x0000, 0x117, BAD_DUMP CRC(55b86dd5) SHA1(4a05d86002f929f38171282d2aad5d986283c45b) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal16v8.ic10",      0x0000, 0x117, BAD_DUMP CRC(23070765) SHA1(3c87f7ab299fcd62da366fc9c1985ab21ee14960) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal20v8.ic18",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_9-2_gal16v8.ic42",      0x0000, 0x117, BAD_DUMP CRC(d62f0a0d) SHA1(76bd995c8b71b12b752ed517d87a78e55a8bab00) ) // Bruteforced
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46", 0x0000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace

// Xor World was originally developed using the Modular System, so this isn't a bootleg
// however the ROMs here contain the title logos from the 'xorworld' sets but don't
// make use of them, instead opting for their own, so which really came first?

// due to this being almost an entirely different piece of code on entirely different hardware it isn't set as a clone of xorworld
GAME( 1990, xorwldms,  0,        xorworld_ms, xorworld_ms, xorworld_ms_state, init_xorworld_ms, ROT0, "Gaelco", "Xor World (Modular System, prototype, set 1)", MACHINE_IMPERFECT_SOUND )
GAME( 1990, xorwldmsa, xorwldms, xorworld_ms, xorworld_ms, xorworld_ms_state, init_xorworld_ms, ROT0, "Gaelco", "Xor World (Modular System, prototype, set 2)", MACHINE_IMPERFECT_SOUND )
