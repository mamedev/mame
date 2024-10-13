// license:BSD-3-Clause
// copyright-holders: Bryan McPhail

/***************************************************************************

    Lemmings                (c) 1991 Data East USA (DE-0357)

    Prototype!  Licensed from the home computer version this game never
    made it past the arcade field test stage.  Unlike most Data East games
    this hardware features a pixel layer and a VRAM layer, probably to
    make the transition from the pixel addressable computer code to the
    arcade hardware.

    As prototype software it seems to have a couple of non-critical bugs,
    the palette RAM check and VRAM check both overrun their actual RAM size.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"

#include "deco146.h"
#include "decospr.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lemmings_state : public driver_device
{
public:
	lemmings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bitmap0(2048, 256)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco146(*this, "ioprot")
		, m_spriteram(*this, "spriteram%u", 1)
		, m_sprgen(*this, "spritegen%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_control_data(*this, "control_data")
		, m_vram_data(*this, "vram_data")
		, m_pixel_data(*this, "pixel_%u_data", 0)
		, m_trackball_io(*this, "AN%u", 0)
	{
	}

	void lemmings(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// video-related
	bitmap_ind16 m_bitmap0{};
	tilemap_t *m_vram_tilemap = nullptr;
	std::unique_ptr<uint16_t[]> m_sprite_triple_buffer[2]{};
	std::unique_ptr<uint8_t[]> m_vram_buffer{};

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco146_device> m_deco146;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	required_device_array<decospr_device, 2> m_sprgen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_control_data;
	required_shared_ptr<uint16_t> m_vram_data;
	required_shared_ptr_array<uint16_t, 2> m_pixel_data;

	required_ioport_array<4> m_trackball_io;

	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t trackball_r(offs_t offset);
	void pixel_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pixel_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void copy_bitmap(bitmap_rgb32 &bitmap, int *xscroll, int *yscroll, const rectangle &cliprect);

	uint16_t protection_region_0_146_r(offs_t offset);
	void protection_region_0_146_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/**************************************************************************

    There are two sets of sprites, the combination of custom chips 52 & 71.
    There is a background pixel layer implemented with discrete logic
    rather than a custom chip and a foreground VRAM tilemap layer that the
    game mostly uses as a pixel layer (the VRAM format is arranged as
    sequential pixels, rather than sequential characters).

***************************************************************************/

TILE_GET_INFO_MEMBER(lemmings_state::get_tile_info)
{
	uint16_t const tile = m_vram_data[tile_index];

	tileinfo.set(0,
			tile & 0x7ff,
			(tile >> 12) & 0xf,
			0);
}

void lemmings_state::video_start()
{
	m_vram_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lemmings_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	m_vram_tilemap->set_transparent_pen(0);
	m_bitmap0.fill(0x100);

	m_vram_buffer = make_unique_clear<uint8_t[]>(2048 * 64); // 64 bytes per VRAM character
	m_gfxdecode->gfx(0)->set_source(m_vram_buffer.get());

	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();

	m_sprite_triple_buffer[0] = make_unique_clear<uint16_t[]>(0x800 / 2);
	m_sprite_triple_buffer[1] = make_unique_clear<uint16_t[]>(0x800 / 2);

	save_item(NAME(m_bitmap0));
	save_pointer(NAME(m_vram_buffer), 2048 * 64);
	save_pointer(NAME(m_sprite_triple_buffer[0]), 0x800 / 2, 0);
	save_pointer(NAME(m_sprite_triple_buffer[1]), 0x800 / 2, 1);
}

void lemmings_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		for (int chip = 0; chip < 2; chip++)
			std::copy_n(&m_spriteram[chip]->buffer()[0], 0x800 / 2, &m_sprite_triple_buffer[chip][0]);
	}
}

/******************************************************************************/

// RAM based
void lemmings_state::pixel_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const old = m_pixel_data[0][offset];
	COMBINE_DATA(&m_pixel_data[0][offset]);
	int const src = m_pixel_data[0][offset];
	if (old == src)
		return;

	int const sy = (offset << 1) >> 11;
	int const sx = (offset << 1) & 0x7ff;

	if (sx > 2047 || sy > 255)
		return;

	m_bitmap0.pix(sy, sx + 0) = ((src >> 8) & 0xf) | 0x100;
	m_bitmap0.pix(sy, sx + 1) = ((src >> 0) & 0xf) | 0x100;
}

// RAM based tiles for the FG tilemap
void lemmings_state::pixel_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pixel_data[1][offset]);
	int const src = m_pixel_data[1][offset];

	int const sy = (offset << 1) >> 9;
	int sx = (offset << 1) & 0x1ff;

	// Copy pixel to buffer for easier decoding later
	int const tile = ((sx / 8) * 32) + (sy / 8);
	m_gfxdecode->gfx(0)->mark_dirty(tile);
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 8) & 0xf;

	sx += 1; // Update both pixels in the word
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 0) & 0xf;
}

void lemmings_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_data[offset]);
	m_vram_tilemap->mark_tile_dirty(offset);
}


void lemmings_state::copy_bitmap(bitmap_rgb32 &bitmap, int *xscroll, int *yscroll, const rectangle &cliprect)
{
	pen_t const *const paldata = m_palette->pens();

	for (int y = cliprect.top(); y < cliprect.bottom(); y++)
	{
		uint32_t *const dst = &bitmap.pix(y, 0);

		for (int x = cliprect.left(); x < cliprect.right(); x++)
		{
			uint16_t const src = m_bitmap0.pix((y - *yscroll) & 0xff, (x - *xscroll) & 0x7ff);

			if (src != 0x100)
				dst[x] = paldata[src];
		}
	}
}

uint32_t lemmings_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x1 = -m_control_data[0];
	int x0 = -m_control_data[2];
	int y = 0;
	rectangle rect(0, 0, cliprect.top(), cliprect.bottom());

	// sprites are flipped relative to tilemaps
	m_sprgen[0]->set_flip_screen(true);
	m_sprgen[1]->set_flip_screen(true);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_sprite_triple_buffer[1].get(), 0x400);
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_sprite_triple_buffer[0].get(), 0x400);

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x300, 0xff);

	// Pixel layer can be windowed in hardware (two player mode)
	if ((m_control_data[6] & 2) == 0)
	{
		copy_bitmap(bitmap, &x1, &y, cliprect);
	}
	else
	{
		rect.setx(0, 159);
		copy_bitmap(bitmap, &x0, &y, rect);

		rect.setx(160, 319);
		copy_bitmap(bitmap, &x1, &y, rect);
	}

	m_sprgen[1]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x200, 0xff);
	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x300, 0xff);
	m_vram_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen[1]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x200, 0xff);
	return 0;
}


void lemmings_state::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// Offset==0 Pixel layer X scroll
	if (offset == 4)
		return; // Watchdog or IRQ ack
	COMBINE_DATA(&m_control_data[offset]);
}

uint16_t lemmings_state::trackball_r(offs_t offset)
{
	if ((offset & 2) == 0)
		return m_trackball_io[(offset & 1) | ((offset & 4) >> 1)]->read();
	return 0;
}

uint16_t lemmings_state::protection_region_0_146_r(offs_t offset)
{
	int const real_address = 0 + (offset *2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t const data = m_deco146->read_data(deco146_addr, cs);
	return data;
}

void lemmings_state::protection_region_0_146_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const real_address = 0 + (offset *2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco146->write_data(deco146_addr, data, mem_mask, cs);
}


/******************************************************************************/

void lemmings_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x120000, 0x1207ff).ram().share("spriteram1");
	map(0x140000, 0x1407ff).ram().share("spriteram2");
	map(0x160000, 0x160fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x170000, 0x17000f).ram().w(FUNC(lemmings_state::control_w)).share(m_control_data);
	map(0x190000, 0x19000f).r(FUNC(lemmings_state::trackball_r));
	map(0x1a0000, 0x1a3fff).rw(FUNC(lemmings_state::protection_region_0_146_r), FUNC(lemmings_state::protection_region_0_146_w)).share("prot16ram"); // Protection device
	map(0x1c0000, 0x1c0001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)); // 1 written once a frame
	map(0x1e0000, 0x1e0001).w(m_spriteram[1], FUNC(buffered_spriteram16_device::write)); // 1 written once a frame
	map(0x200000, 0x201fff).ram().w(FUNC(lemmings_state::vram_w)).share(m_vram_data);
	map(0x202000, 0x202fff).ram();
	map(0x300000, 0x37ffff).ram().w(FUNC(lemmings_state::pixel_0_w)).share(m_pixel_data[0]);
	map(0x380000, 0x39ffff).ram().w(FUNC(lemmings_state::pixel_1_w)).share(m_pixel_data[1]);
}

/******************************************************************************/

void lemmings_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x1000, 0x1000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1800, 0x1800).r(m_deco146, FUNC(deco_146_base_device::soundlatch_r)).nopw(); // writes an extra irq ack?
	map(0x8000, 0xffff).rom();
}

/******************************************************************************/

static INPUT_PORTS_START( lemmings )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Hurry")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Select")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Hurry")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_LOW)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")


	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Credits for 1 Player" )
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, "Credits for 2 Player" )
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, "Credits for Continue" )
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPUNKNOWN( 0x4000, 0x4000 )
	PORT_DIPUNKNOWN( 0x8000, 0x8000 )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	2048,
	4,
	{ STEP4(4,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,8) },
	{ STEP8(8*4*16,1), STEP8(0,1) },
	{ STEP16(0,8*4) },
	16*16*4
};

static GFXDECODE_START( gfx_lemmings )
	GFXDECODE_ENTRY( nullptr,    0, charlayout,     0,   16 ) // Dynamically modified
GFXDECODE_END

static GFXDECODE_START( gfx_lemmings_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, sprite_layout,  512, 16 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_lemmings_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, sprite_layout,  768, 16 ) // Sprites 16x16
GFXDECODE_END

/******************************************************************************/

void lemmings_state::lemmings(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 28_MHz_XTAL / 2); // Data East 59
	m_maincpu->set_addrmap(AS_PROGRAM, &lemmings_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(lemmings_state::irq6_line_hold));

	MC6809E(config, m_audiocpu, 32.22_MHz_XTAL / 24); // MC68B09EP; clock not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &lemmings_state::sound_map);

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(lemmings_state::screen_update));
	screen.screen_vblank().set(FUNC(lemmings_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lemmings);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 1024);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_lemmings_spr2);
	DECO_SPRITE(config, m_sprgen[1], 0, m_palette, gfx_lemmings_spr1);

	DECO146PROT(config, m_deco146, 0);
	m_deco146->port_a_cb().set_ioport("INPUTS");
	m_deco146->port_b_cb().set_ioport("SYSTEM");
	m_deco146->port_c_cb().set_ioport("DSW");
	m_deco146->set_use_magic_read_address_xor(true);
	m_deco146->soundlatch_irq_cb().set_inputline(m_audiocpu, M6809_FIRQ_LINE);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 32.22_MHz_XTAL / 9)); // clock likely wrong
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6809_IRQ_LINE);
	ymsnd.add_route(0, "lspeaker", 0.45);
	ymsnd.add_route(1, "rspeaker", 0.45);

	okim6295_device &oki(OKIM6295(config, "oki", 1023924, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}

/******************************************************************************/

ROM_START( lemmings )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "lemmings.5", 0x00000, 0x20000, CRC(e9a2b439) SHA1(873723a06d71bb41772951f451a75578b30267d5) )
	ROM_LOAD16_BYTE( "lemmings.1", 0x00001, 0x20000, CRC(bf52293b) SHA1(47a1ed64bf02776db086fdce80997b8a0c068791) )
	ROM_LOAD16_BYTE( "lemmings.6", 0x40000, 0x20000, CRC(0e3dc0ea) SHA1(533abf66ca4b578d03566d5de922dc5828c26eca) )
	ROM_LOAD16_BYTE( "lemmings.2", 0x40001, 0x20000, CRC(0cf3d7ce) SHA1(95dc43a8cded860fcf8743b62cbe4f2a97f43215) )
	ROM_LOAD16_BYTE( "lemmings.7", 0x80000, 0x20000, CRC(d020219c) SHA1(9678d8636798d1e528269fe2f9eb532e189c134e) )
	ROM_LOAD16_BYTE( "lemmings.3", 0x80001, 0x20000, CRC(c635494a) SHA1(e105dc79bd3c425d971629a3066c38dbf08b6428) )
	ROM_LOAD16_BYTE( "lemmings.8", 0xc0000, 0x20000, CRC(9166ce09) SHA1(7f0970cc07ebdbfc9a738342259d07d37b397161) )
	ROM_LOAD16_BYTE( "lemmings.4", 0xc0001, 0x20000, CRC(aa845488) SHA1(d17ec80f43d2a0123e93fad83d4e1319eb18d7c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "lemmings.15",    0x00000, 0x10000, CRC(f0b24a35) SHA1(1aaeb1e6faee04d2e433161fd7aa965b58e3b8a7) )

	ROM_REGION( 0x40000, "sprites1", ROMREGION_ERASE00 ) // 3bpp data but sprite chip expects 4
	ROM_LOAD32_BYTE( "lemmings.9",  0x000003, 0x10000, CRC(e06442f5) SHA1(d9c8b681cce1d0257a0446bc820c7d679e2a1168) )
	ROM_LOAD32_BYTE( "lemmings.10", 0x000002, 0x10000, CRC(36398848) SHA1(6c6956607f889c35367e6df4a32359042fad695e) )
	ROM_LOAD32_BYTE( "lemmings.11", 0x000001, 0x10000, CRC(b46a54e5) SHA1(53b053346f80357aecff4ab888a8562f99cb318f) )

	ROM_REGION( 0x40000, "sprites2", ROMREGION_ERASE00 ) // 3bpp data but sprite chip expects 4
	ROM_LOAD32_BYTE( "lemmings.12", 0x000003, 0x10000, CRC(dc9047ff) SHA1(1bbe573fa51127a9e8b970a353f3cceab00f486a) )
	ROM_LOAD32_BYTE( "lemmings.13", 0x000002, 0x10000, CRC(7cc15491) SHA1(73c1c11b2738f6679c70cae8ac4c55cdc9b8fc27) )
	ROM_LOAD32_BYTE( "lemmings.14", 0x000001, 0x10000, CRC(c162788f) SHA1(e1f669efa59699cd1b7da71b112701ee79240c18) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "lemmings.16",    0x00000, 0x20000, CRC(f747847c) SHA1(00880fa6dff979e5d15daea61938bd18c768c92f) )
ROM_END

} // anonymous namespace


/******************************************************************************/

GAME( 1991, lemmings, 0, lemmings, lemmings, lemmings_state, empty_init, ROT0, "Data East USA", "Lemmings (US prototype)", MACHINE_SUPPORTS_SAVE )
