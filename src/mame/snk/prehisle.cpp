// license:BSD-3-Clause
// copyright-holders: Bryan McPhail

/***************************************************************************

    Prehistoric Isle in 1930 (World)        (c) 1989 SNK
    Prehistoric Isle in 1930 (USA)          (c) 1989 SNK
    原始島(Wonsido) 1930's (Korea)           (c) 1989 SNK / Victor
    原始島(Genshitō) 1930's (Japan)          (c) 1989 SNK

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class prehisle_state : public driver_device
{
public:
	prehisle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tx_vram(*this, "tx_vram"),
		m_spriteram(*this, "spriteram"),
		m_fg_vram(*this, "fg_vram"),
		m_tilemap_rom(*this, "bgtilemap"),
		m_io_p1(*this, "P1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void prehisle(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	void soundcmd_w(u16 data);
	void fg_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tx_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_scrolly_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_scrollx_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_scrolly_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_scrollx_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void upd_port_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_program_map(address_map &map);

	required_shared_ptr<u16> m_tx_vram;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_fg_vram;
	required_region_ptr<u8> m_tilemap_rom;

	required_ioport m_io_p1;
	u8 m_invert_controls = 0;
	u16 m_bg_scrollx = 0;
	u16 m_bg_scrolly = 0;
	u16 m_fg_scrollx = 0;
	u16 m_fg_scrolly = 0;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};


void prehisle_state::fg_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_vram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void prehisle_state::tx_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_vram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

void prehisle_state::fg_scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_scrolly);
	m_fg_tilemap->set_scrolly(0, m_fg_scrolly);
}

void prehisle_state::fg_scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_scrollx);
	m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
}

void prehisle_state::bg_scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_scrolly);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
}

void prehisle_state::bg_scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_scrollx);
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
}


/* tile layout
0  xxxx....  color
0  ....x...  flip x
0  .....xxx  gfx code high bits
1  xxxxxxxx  gfx code low bits
*/
TILE_GET_INFO_MEMBER(prehisle_state::get_bg_tile_info)
{
	int const offs = tile_index * 2;
	int const attr = m_tilemap_rom[offs + 1] + (m_tilemap_rom[offs] << 8);
	int const code = attr & 0x7ff;
	int const color = attr >> 12;
	int const flags = (attr & 0x800) ? TILE_FLIPX : 0;

	tileinfo.set(1, code, color, flags);
}

/* tile layout
0  xxxx.... ........  color
0  ....x... ........  flip y
0  .....xxx xxxxxxxx  gfx code
*/
TILE_GET_INFO_MEMBER(prehisle_state::get_fg_tile_info)
{
	int const attr = m_fg_vram[tile_index];
	int const code = attr & 0x7ff;
	int const color = attr >> 12;
	int const flags = (attr & 0x800) ? TILE_FLIPY : 0;

	tileinfo.set(2, code, color, flags);
}

/* tile layout
0  xxxx.... ........  color
0  ....xxxx xxxxxxxx  gfx code
*/
TILE_GET_INFO_MEMBER(prehisle_state::get_tx_tile_info)
{
	int const attr = m_tx_vram[tile_index];
	int const code = attr & 0xfff;
	int const color = attr >> 12;

	tileinfo.set(0, code, color, 0);
}

void prehisle_state::video_start()
{
	// ROM-based background layer
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(prehisle_state::get_bg_tile_info)),
			TILEMAP_SCAN_COLS,      // scan order
			16, 16,                 // tile size
			1024, 32);              // tilemap size

	// RAM-based foreground layer (overlays most sprites)
	m_fg_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(prehisle_state::get_fg_tile_info)),
			TILEMAP_SCAN_COLS,      // scan order
			16, 16,                 // tile size
			256, 32);               // tilemap size
	m_fg_tilemap->set_transparent_pen(15);

	// text layer
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(prehisle_state::get_tx_tile_info)),
			TILEMAP_SCAN_ROWS,      // scan order
			8, 8,                   // tile size
			32, 32);                // tilemap size
	m_tx_tilemap->set_transparent_pen(15);

	// register for saving
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
}

/* sprite layout

0  .......x xxxxxxxx  y, other bits unused?
1  .......x xxxxxxxx  x, other bits unused?
2  x....... ........  flip y
2  .x...... ........  flip x
2  ..x..... ........  ?
2  ...xxxxx xxxxxxxx  gfx code
3  xxxx.... ........  color+priority, other bits unknown
*/
void prehisle_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 1024 - 4; offs >= 0; offs -= 4)
	{
		u16 const attr = m_spriteram[offs + 2];
		u16 const code = attr & 0x1fff;
		u16 const color = m_spriteram[offs + 3] >> 12;
		u32 const priority = GFX_PMASK_4 | ((color < 0x4) ? 0 : GFX_PMASK_2); // correct?
		bool flipx = attr & 0x4000;
		bool flipy = attr & 0x8000;
		s16 sx = m_spriteram[offs + 1] & 0x1ff;
		s16 sy = m_spriteram[offs] & 0x1ff;

		// coordinates are 9-bit signed
		if (sx & 0x100) sx = -0x100 + (sx & 0xff);
		if (sy & 0x100) sy = -0x100 + (sy & 0xff);

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(3)->prio_transpen(
			bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			screen.priority(), priority,
			15); // transparent pen
	}
}

u32 prehisle_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);
	draw_sprites(screen, bitmap, cliprect);
	return 0;
}


/******************************************************************************/

void prehisle_state::soundcmd_w(u16 data)
{
	m_soundlatch->write(data & 0xff);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/*******************************************************************************/

void prehisle_state::main_program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x070000, 0x073fff).ram();
	map(0x090000, 0x0907ff).ram().w(FUNC(prehisle_state::tx_vram_w)).share(m_tx_vram);
	map(0x0a0000, 0x0a07ff).ram().share(m_spriteram);
	map(0x0b0000, 0x0b3fff).ram().w(FUNC(prehisle_state::fg_vram_w)).share(m_fg_vram);
	map(0x0d0000, 0x0d07ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0e0010, 0x0e0011).portr("P2");
	map(0x0e0020, 0x0e0021).portr("COIN");
	map(0x0e0041, 0x0e0041).lr8(NAME([this] () -> u8 { return m_io_p1->read() ^ m_invert_controls; }));
	map(0x0e0042, 0x0e0043).portr("DSW0");
	map(0x0e0044, 0x0e0045).portr("DSW1"); // and VBLANK
	map(0x0f0000, 0x0f0001).w(FUNC(prehisle_state::fg_scrolly_w));
	map(0x0f0010, 0x0f0011).w(FUNC(prehisle_state::fg_scrollx_w));
	map(0x0f0020, 0x0f0021).w(FUNC(prehisle_state::bg_scrolly_w));
	map(0x0f0030, 0x0f0031).w(FUNC(prehisle_state::bg_scrollx_w));
	map(0x0f0046, 0x0f0047).lw16(NAME([this] (u16 data) { m_invert_controls = data ? 0xff : 0x00; }));
	map(0x0f0050, 0x0f0051).lw16(NAME([this] (u16 data) { machine().bookkeeping().coin_counter_w(0, data & 1); }));
	map(0x0f0052, 0x0f0053).lw16(NAME([this] (u16 data) { machine().bookkeeping().coin_counter_w(1, data & 1); }));
	map(0x0f0060, 0x0f0061).lw16(NAME([this] (u16 data) { flip_screen_set(data & 0x01); }));
	map(0x0f0070, 0x0f0071).w(FUNC(prehisle_state::soundcmd_w));
}

/******************************************************************************/

void prehisle_state::upd_port_w(u8 data)
{
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

void prehisle_state::sound_program_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf800, 0xf800).nopw();    // ???
}

void prehisle_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x40, 0x40).w(FUNC(prehisle_state::upd_port_w));
	map(0x80, 0x80).lw8(NAME([this] (u8 data) { m_upd7759->reset_w(BIT(data, 7)); }));
}

/******************************************************************************/

static INPUT_PORTS_START( prehisle )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Only Twice" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "A 4C/1C B 1C/4C" )
	PORT_DIPSETTING(    0x10, "A 3C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x20, "A 2C/1C B 1C/2C" )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Standard ) )
	PORT_DIPSETTING(    0x01, "Middle" )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )             PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Infinite Lives" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "100K 200K" )
	PORT_DIPSETTING(    0x20, "150K 300K" )
	PORT_DIPSETTING(    0x10, "300K 500K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

/******************************************************************************/

static GFXDECODE_START( gfx_prehisle )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_msb,                 0, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x4_col_2x2_group_packed_msb, 768, 16 )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_col_2x2_group_packed_msb, 512, 16 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_col_2x2_group_packed_msb, 256, 16 )
GFXDECODE_END

/******************************************************************************/

void prehisle_state::machine_start()
{
	// register for saving
	save_item(NAME(m_invert_controls));
}

/******************************************************************************/

void prehisle_state::prehisle(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(18'000'000) / 2);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &prehisle_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(prehisle_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(4'000'000));    // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &prehisle_state::sound_program_map);
	m_audiocpu->set_addrmap(AS_IO, &prehisle_state::sound_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// the screen parameters are guessed but should be accurate. They
	// give a theoretical refresh rate of 59.1856Hz while the measured
	// rate on a snk68.c with very similar hardware board is 59.16Hz.
	screen.set_raw(XTAL(24'000'000) / 4, 384, 0, 256, 264, 16, 240);
	screen.set_screen_update(FUNC(prehisle_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_prehisle);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(4'000'000)));  // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.90);
}

/******************************************************************************/

ROM_START( prehisle )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gt-e2.2h", 0x00000, 0x20000, CRC(7083245a) SHA1(c4f72440e3fb130c8c44224c958bf70c61e8c34e) ) // red "E" stamped on printed label
	ROM_LOAD16_BYTE( "gt-e3.3h", 0x00001, 0x20000, CRC(6d8cdf58) SHA1(0078e54db899132d2b1244aed0b974173717f82e) ) // red "E" stamped on printed label

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gt1.1",  0x000000, 0x10000, CRC(80a4c093) SHA1(abe59e43259eb80b504bd5541f58cd0e5eb998ab) )

	ROM_REGION( 0x008000, "chars", 0 )
	ROM_LOAD( "gt15.b15",   0x000000, 0x08000, CRC(ac652412) SHA1(916c04c3a8a7bfb961313ab73c0a27d7f5e48de1) )

	ROM_REGION( 0x040000, "bgtiles", 0 )
	ROM_LOAD( "pi8914.b14", 0x000000, 0x40000, CRC(207d6187) SHA1(505dfd1424b894e7b898f91b89f021ddde433c48) )

	ROM_REGION( 0x040000, "fgtiles", 0 )
	ROM_LOAD( "pi8916.h16", 0x000000, 0x40000, CRC(7cffe0f6) SHA1(aba08617964fc425418b098be5167021768bd47c) )

	ROM_REGION( 0x0a0000, "sprites", 0 )
	ROM_LOAD( "pi8910.k14", 0x000000, 0x80000, CRC(5a101b0b) SHA1(9645ab1f8d058cf2c6c42ccb4ce92a9b5db10c51) )
	ROM_LOAD( "gt5.5",      0x080000, 0x20000, CRC(3d3ab273) SHA1(b5706ada9eb2c22fcc0ac8ede2d2ee02ee853191) )

	ROM_REGION( 0x10000, "bgtilemap", 0 )
	ROM_LOAD( "gt11.11",  0x000000, 0x10000, CRC(b4f0fcf0) SHA1(b81cc0b6e3e6f5616789bb3e77807dc0ef718a38) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "gt4.4",  0x000000, 0x20000, CRC(85dfb9ec) SHA1(78c865e7ccffddb71dcddccab358fa945f521f25) )
ROM_END

ROM_START( prehisleu )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gt-u2.2h", 0x00000, 0x20000, CRC(a14f49bb) SHA1(6b39a894c3d3862be349a58c748d2d763d5a269c) ) // red "U" stamped on printed label
	ROM_LOAD16_BYTE( "gt-u3.3h", 0x00001, 0x20000, CRC(f165757e) SHA1(26cf369fed1713deec182852d76fe014ed46d6ac) ) // red "U" stamped on printed label

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gt1.1",  0x000000, 0x10000, CRC(80a4c093) SHA1(abe59e43259eb80b504bd5541f58cd0e5eb998ab) )

	ROM_REGION( 0x008000, "chars", 0 )
	ROM_LOAD( "gt15.b15",   0x000000, 0x08000, CRC(ac652412) SHA1(916c04c3a8a7bfb961313ab73c0a27d7f5e48de1) )

	ROM_REGION( 0x040000, "bgtiles", 0 )
	ROM_LOAD( "pi8914.b14", 0x000000, 0x40000, CRC(207d6187) SHA1(505dfd1424b894e7b898f91b89f021ddde433c48) )

	ROM_REGION( 0x040000, "fgtiles", 0 )
	ROM_LOAD( "pi8916.h16", 0x000000, 0x40000, CRC(7cffe0f6) SHA1(aba08617964fc425418b098be5167021768bd47c) )

	ROM_REGION( 0x0a0000, "sprites", 0 )
	ROM_LOAD( "pi8910.k14", 0x000000, 0x80000, CRC(5a101b0b) SHA1(9645ab1f8d058cf2c6c42ccb4ce92a9b5db10c51) )
	ROM_LOAD( "gt5.5",      0x080000, 0x20000, CRC(3d3ab273) SHA1(b5706ada9eb2c22fcc0ac8ede2d2ee02ee853191) )

	ROM_REGION( 0x10000, "bgtilemap", 0 )
	ROM_LOAD( "gt11.11",  0x000000, 0x10000, CRC(b4f0fcf0) SHA1(b81cc0b6e3e6f5616789bb3e77807dc0ef718a38) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "gt4.4",  0x000000, 0x20000, CRC(85dfb9ec) SHA1(78c865e7ccffddb71dcddccab358fa945f521f25) )
ROM_END

ROM_START( prehislek )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gt-k2.2h", 0x00000, 0x20000, CRC(f2d3544d) SHA1(28d41a81ac12ef951610ba0aa70945c069d69d75) ) // red "K" stamped on printed label
	ROM_LOAD16_BYTE( "gt-k3.3h", 0x00001, 0x20000, CRC(ebf7439b) SHA1(76fcad47bc8ae371ecf265fd378e2c4856d39c7f) ) // red "K" stamped on printed label

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gt1.1",  0x000000, 0x10000, CRC(80a4c093) SHA1(abe59e43259eb80b504bd5541f58cd0e5eb998ab) )

	ROM_REGION( 0x008000, "chars", 0 )
	ROM_LOAD( "gt15k.b15",   0x000000, 0x08000, CRC(4cad1451) SHA1(5c81240e94f5a38af874b2ec7c65cb0f55d2ba8c) )

	ROM_REGION( 0x040000, "bgtiles", 0 )
	ROM_LOAD( "pi8914.b14", 0x000000, 0x40000, CRC(207d6187) SHA1(505dfd1424b894e7b898f91b89f021ddde433c48) )

	ROM_REGION( 0x040000, "fgtiles", 0 )
	ROM_LOAD( "pi8916.h16", 0x000000, 0x40000, CRC(7cffe0f6) SHA1(aba08617964fc425418b098be5167021768bd47c) )

	ROM_REGION( 0x0a0000, "sprites", 0 )
	ROM_LOAD( "pi8910.k14", 0x000000, 0x80000, CRC(5a101b0b) SHA1(9645ab1f8d058cf2c6c42ccb4ce92a9b5db10c51) )
	ROM_LOAD( "gt5.5",      0x080000, 0x20000, CRC(3d3ab273) SHA1(b5706ada9eb2c22fcc0ac8ede2d2ee02ee853191) )

	ROM_REGION( 0x10000, "bgtilemap", 0 )
	ROM_LOAD( "gt11.11",  0x000000, 0x10000, CRC(b4f0fcf0) SHA1(b81cc0b6e3e6f5616789bb3e77807dc0ef718a38) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "gt4.4",  0x000000, 0x20000, CRC(85dfb9ec) SHA1(78c865e7ccffddb71dcddccab358fa945f521f25) )
ROM_END

ROM_START( gensitou )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gt-j2.2h", 0x00000, 0x20000, CRC(a2da0b6b) SHA1(d102118f83b96094fd4ea4b3468713c4946c949d) ) // red "J" stamped on printed label
	ROM_LOAD16_BYTE( "gt-j3.3h", 0x00001, 0x20000, CRC(c1a0ae8e) SHA1(2c9643abfd71edf8612e63d69cea4fbc19aad19d) ) // red "J" stamped on printed label

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gt1.1",  0x000000, 0x10000, CRC(80a4c093) SHA1(abe59e43259eb80b504bd5541f58cd0e5eb998ab) )

	ROM_REGION( 0x008000, "chars", 0 )
	ROM_LOAD( "gt15.b15",   0x000000, 0x08000, CRC(ac652412) SHA1(916c04c3a8a7bfb961313ab73c0a27d7f5e48de1) )

	ROM_REGION( 0x040000, "bgtiles", 0 )
	ROM_LOAD( "pi8914.b14", 0x000000, 0x40000, CRC(207d6187) SHA1(505dfd1424b894e7b898f91b89f021ddde433c48) )

	ROM_REGION( 0x040000, "fgtiles", 0 )
	ROM_LOAD( "pi8916.h16", 0x000000, 0x40000, CRC(7cffe0f6) SHA1(aba08617964fc425418b098be5167021768bd47c) )

	ROM_REGION( 0x0a0000, "sprites", 0 )
	ROM_LOAD( "pi8910.k14", 0x000000, 0x80000, CRC(5a101b0b) SHA1(9645ab1f8d058cf2c6c42ccb4ce92a9b5db10c51) )
	ROM_LOAD( "gt5.5",      0x080000, 0x20000, CRC(3d3ab273) SHA1(b5706ada9eb2c22fcc0ac8ede2d2ee02ee853191) )

	ROM_REGION( 0x10000, "bgtilemap", 0 )
	ROM_LOAD( "gt11.11",  0x000000, 0x10000, CRC(b4f0fcf0) SHA1(b81cc0b6e3e6f5616789bb3e77807dc0ef718a38) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "gt4.4",  0x000000, 0x20000, CRC(85dfb9ec) SHA1(78c865e7ccffddb71dcddccab358fa945f521f25) )
ROM_END

// world bootleg using 64k*8 UVEPROMs, program and sound unchanged, sprites and background tilemaps altered
ROM_START( prehisleb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u_h1.bin", 0x00000, 0x10000, CRC(04c1703b) SHA1(36aa9b8cd11621faa094af4dd2fb5a0e59318a5e) )
	ROM_LOAD16_BYTE( "u_h3.bin", 0x00001, 0x10000, CRC(62f04cd1) SHA1(9506e18a7847362128e06781e783fdb1f562e502) )
	ROM_LOAD16_BYTE( "u_j2.bin", 0x20000, 0x10000, CRC(7b12501d) SHA1(678d32f70d86807449ffe617c7c6e257d308d8af) )
	ROM_LOAD16_BYTE( "u_j3.bin", 0x20001, 0x10000, CRC(2a86f7c4) SHA1(5bca393f6edfcd41e1803ea1062497752fd400a9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u_e12.bin", 0x00000, 0x10000, CRC(80a4c093) SHA1(abe59e43259eb80b504bd5541f58cd0e5eb998ab) )

	ROM_REGION( 0x008000, "chars", 0 )
	ROM_LOAD( "l_a17.bin", 0x00000, 0x08000, CRC(ac652412) SHA1(916c04c3a8a7bfb961313ab73c0a27d7f5e48de1) )

	ROM_REGION( 0x040000, "bgtiles", 0 )
	ROM_LOAD( "l_b17.bin", 0x00000, 0x10000, CRC(65a22ffc) SHA1(0122c15f9c948bd6a12d44f71178d0a8b7f38c2e) )
	ROM_LOAD( "l_b16.bin", 0x10000, 0x10000, CRC(b1e1f527) SHA1(07d88d4a1f198bd5e37dcb1521904c5f8d851f4d) )
	ROM_LOAD( "l_b14.bin", 0x20000, 0x10000, CRC(28e94d40) SHA1(e77187040b5c5c7354088aa1173b23493cf26b78) )
	ROM_LOAD( "l_b13.bin", 0x30000, 0x10000, CRC(4dbb557a) SHA1(af07074dae121264018f2f6f3489cce243bfd3c0) )

	ROM_REGION( 0x040000, "fgtiles", 0 )
	ROM_LOAD( "l_h17.bin", 0x00000, 0x10000, CRC(79c42316) SHA1(7a1c72c9146ce50d9c24ec4f3ae210103a95c2eb) )
	ROM_LOAD( "l_h15.bin", 0x10000, 0x10000, CRC(50e31fb0) SHA1(043181041354e3af07b6b32fc6192aae9e49d869) )
	ROM_LOAD( "l_f17.bin", 0x20000, 0x10000, CRC(2af1739d) SHA1(e17b88ee525247100b038f2200ad5a1ce4e71cb2) )
	ROM_LOAD( "l_f15.bin", 0x30000, 0x10000, CRC(cac11327) SHA1(c0feb6f3d9b8bba1dab66142fa44269bda579443) )

	ROM_REGION( 0x0a0000, "sprites", 0 )
	ROM_LOAD( "u_k12.bin", 0x00000, 0x10000, CRC(4b0215f0) SHA1(340e68e9b9603829a200ad1ff7c0b373d39ca4dc) )
	ROM_LOAD( "u_k13.bin", 0x10000, 0x10000, CRC(68b8a698) SHA1(ff87c47cb600bacdb50b2e8ad87090a0e0146d12) )
	ROM_LOAD( "u_j4.bin",  0x20000, 0x10000, CRC(06ce7b57) SHA1(d19f35405b34bb43a2ca341c020c14de4c8474d6) )
	ROM_LOAD( "u_j5.bin",  0x30000, 0x10000, CRC(2ee8b401) SHA1(6f4a3ff75daae790872477a600c9e61332f74a46) )
	ROM_LOAD( "u_j7.bin",  0x40000, 0x10000, CRC(35656cbc) SHA1(bed0b2bfb9bd8487718a14d5388c61740d0e0e3a) )
	ROM_LOAD( "u_j8.bin",  0x50000, 0x10000, CRC(1e7e9336) SHA1(28b13ab7e9a0bb806af8fe3dbc2b100b93b29c5c) )
	ROM_LOAD( "u_j10.bin", 0x60000, 0x10000, CRC(785bf046) SHA1(5ab3f883643de6c59e764775b11b275989437fa2) )
	ROM_LOAD( "u_j11.bin", 0x70000, 0x10000, CRC(c306b9fa) SHA1(58c8d64dd7ae80b5d21d289757de442ac8e9264c) )
	ROM_LOAD( "u_j12.bin" ,0x80000, 0x10000, CRC(5ba5bbed) SHA1(6af3503e0277a926815afb973d67c4ad7a0427d1) )
	ROM_LOAD( "u_j13.bin", 0x90000, 0x10000, CRC(007dee47) SHA1(e45ce52a471783864cc2704b3b0462c32ddf7e52) ) // modified by bootleggers

	ROM_REGION( 0x10000, "bgtilemap", 0 )
	ROM_LOAD( "l_a6.bin",  0x00000, 0x10000, CRC(e2b9a44b) SHA1(4a1be44c19a724727218bbdc120bafbbe095747a) ) // modified by bootleggers

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "u_f14.bin", 0x00000, 0x10000, CRC(2fb32933) SHA1(2cea86dfe9a6a0b2de34c3c952c625ad30a7ebea) )
	ROM_LOAD( "u_j14.bin", 0x10000, 0x10000, CRC(32d5f7c9) SHA1(23abc82f83296c62320a047b9f63032a7f07bf6d) )
ROM_END

} // anonymous namespace


/******************************************************************************/

//原始島(Shared Title GFX for Japan and Korea set, JP: げんしとう-Genshitō; KR: 원시도-Wonsido)/Prehistoric Isle in 1930(English)
GAME( 1989, prehisle,  0,        prehisle, prehisle, prehisle_state, empty_init, ROT0, "SNK",                  "Prehistoric Isle in 1930 (World)",          MACHINE_SUPPORTS_SAVE )
GAME( 1989, prehisleu, prehisle, prehisle, prehisle, prehisle_state, empty_init, ROT0, "SNK",                  "Prehistoric Isle in 1930 (US)",             MACHINE_SUPPORTS_SAVE )
GAME( 1989, prehislek, prehisle, prehisle, prehisle, prehisle_state, empty_init, ROT0, "SNK (Victor license)", "Wonsido 1930's (Korea)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1989, gensitou,  prehisle, prehisle, prehisle, prehisle_state, empty_init, ROT0, "SNK",                  "Genshitou 1930's",                          MACHINE_SUPPORTS_SAVE )
GAME( 1989, prehisleb, prehisle, prehisle, prehisle, prehisle_state, empty_init, ROT0, "bootleg",              "Prehistoric Isle in 1930 (World, bootleg)", MACHINE_SUPPORTS_SAVE )
