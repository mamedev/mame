// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
	Splash (Modular System)

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


class splashms_state : public driver_device
{
public:
	splashms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_paletteram(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram")
	{ }

	void splashms(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_soundcpu;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_paletteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_videoram;

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void splashms_map(address_map &map);
	void sub_map(address_map &map);
	void sub_portmap(address_map &map);
	void sound_map(address_map &map);

	uint16_t unknown_r();
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap0);
	tilemap_t *m_bg_tilemap;

};

uint16_t splashms_state::unknown_r()
{
	return machine().rand();
}


void splashms_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(splashms_state::get_tile_info_tilemap0)
{
	int tile = m_videoram[tile_index*2];
	int attr = m_videoram[(tile_index*2)+1] & 0xf;
	tileinfo.set(2,tile + 0x4000,attr,0);
}

uint32_t splashms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void splashms_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(splashms_state::get_tile_info_tilemap0)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
}


void splashms_state::splashms_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x080000, 0x081fff).ram().w(FUNC(splashms_state::vram_w)).share("videoram");

	map(0x082000, 0x0fffff).ram();
	
	map(0x100000, 0x1007ff).ram(); 

	map(0x200000, 0x200fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x400000, 0x400001).portr("IN0");
	map(0x400002, 0x400003).portr("IN1");
	map(0x400004, 0x400005).portr("IN2").nopw();
	map(0x400006, 0x400007).portr("IN3");
	map(0x400008, 0x400009).portr("IN4"); // service mode in here

	map(0x40000c, 0x40000d).portr("IN5");

	map(0x40000e, 0x40000f).nopw();

	map(0xff0000, 0xffffff).ram();
}

void splashms_state::sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).nopw(); // banking for 0x4000-0x7fff RAM?
}

void splashms_state::sub_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x7fff).ram();
}

void splashms_state::sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();

	// sound chip
	map(0xe800, 0xe800).nopr().nopw();
	map(0xe801, 0xe801).nopw();

	map(0xf000, 0xf7ff).ram();
}

void splashms_state::machine_start()
{
}




static INPUT_PORTS_START( splashms )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0400, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END

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

static GFXDECODE_START( gfx_splashms )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8x4_layout, 0, 16 )
GFXDECODE_END

void splashms_state::splashms(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &splashms_state::splashms_map);
	m_maincpu->set_vblank_int("screen", FUNC(splashms_state::irq4_line_hold));

	Z80(config, m_subcpu, 12_MHz_XTAL/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &splashms_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &splashms_state::sub_portmap);

	Z80(config, m_soundcpu, 16_MHz_XTAL/4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &splashms_state::sound_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(splashms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x800);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_splashms);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}

ROM_START( splashms )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cpu1_6-1_sp_608a.ic8",  0x000001, 0x020000, CRC(befdbaf0) SHA1(94efdeec1e1311317ffd0fe3d5fdbb02e151b985) )
	ROM_LOAD16_BYTE( "cpu1_6-1_sp_617a.ic17", 0x000000, 0x020000, CRC(080edb2b) SHA1(6104345bc72cd20051d66c04b97c9a365a88ec3f) )

	ROM_REGION( 0x300000, "subcpu", 0 ) // extra Z80 for backgrounds!
	ROM_LOAD( "cpu2_c_sp_c2.ic2", 0x000000, 0x080000, CRC(3a0be09f) SHA1(83abc10ff2c810c8451f583700f140f569e5b6ee) )
	ROM_LOAD( "cpu2_c_sp_c3.ic3", 0x080000, 0x080000, CRC(c3dc5e9d) SHA1(ce5fb65935cfe225132242e058cd63fa33f9da63) )
	ROM_LOAD( "cpu2_c_sp_c4.ic4", 0x100000, 0x080000, CRC(4d7b643d) SHA1(40bdcf7eedddc3244cb41530d10009b23b7ac473) )
	ROM_LOAD( "cpu2_c_sp_c5.ic5", 0x180000, 0x080000, CRC(7ba31717) SHA1(000cf6ec261ac90efc3e4f2dbf6720a54fb3bbdb) )
	ROM_LOAD( "cpu2_c_sp_c6.ic6", 0x200000, 0x080000, CRC(994e8e16) SHA1(cbe7d9e192d0390b123f1b585a0463634f33b485) )
	ROM_LOAD( "cpu2_c_sp_c7.ic7", 0x280000, 0x080000, CRC(6ea0be42) SHA1(5cc45ef1f3c8f46e1e7b3ad3313d221a65fb0025) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "snd_9-2_sp_916.ic6", 0x000000, 0x010000, CRC(5567fa22) SHA1(3993c733a0222ca292b60f409c78b45280a5fec6) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "8_sp_833.ic33", 0x000000, 0x020000, CRC(8ac7cd1f) SHA1(aad88db9a82f417774f1d5eef830cc97c0d4b0de) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_826.ic26", 0x000001, 0x020000, CRC(b7ec71d8) SHA1(3d4b62559c0ba688b94e605594f3e8e9f2cbefa2) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_818.ic18", 0x000002, 0x020000, CRC(ae62a832) SHA1(f825a186e25a1c292aa6f880055341ec14373c0b) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_811.ic11", 0x000003, 0x020000, CRC(02e474b4) SHA1(11446655cb73ec4961339fa4ee41200f8b2b81d3) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_837.ic37", 0x080000, 0x010000, CRC(3b544131) SHA1(e7fd97cb24b84739f2481efb1d232f86df4a3d8d) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_830.ic30", 0x080001, 0x010000, CRC(09bb675b) SHA1(49c41ccfce1b0077c430c6bb38bc858aeaf87fb8) ) // has some garbage in the blank space of the paired ROMs
	ROM_LOAD32_BYTE( "8_sp_822.ic22", 0x080002, 0x010000, CRC(621fcf26) SHA1(a7ff6b12fbbea1bba7c4a397a82ac2fb5c09558a) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_815.ic15", 0x080003, 0x010000, CRC(5641b621) SHA1(e71df1ab5c9b2254495d99657477b52e8843d128) ) // 1xxxxxxxxxxxxxxx = 0xFF                          
                                   
	ROM_REGION( 0x080000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "5-1_sp_524.ic24", 0x000000, 0x010000, CRC(841c24c1) SHA1(70cb26033999f8184c51849e00bfcb2270f646e8) )
	ROM_LOAD32_BYTE( "5-1_sp_518.ic18", 0x000001, 0x010000, CRC(499cb813) SHA1(4d22e58530ff8a85b7ffc8ae1ab5986215986b49) )
	ROM_LOAD32_BYTE( "5-1_sp_512.ic12", 0x000002, 0x010000, CRC(8cb0b132) SHA1(894f84b6a8171ed8c22298ebf1303da020f747ee) )
	ROM_LOAD32_BYTE( "5-1_sp_503.ic3",  0x000003, 0x010000, CRC(ace09666) SHA1(d223718118b9912643d320832414df942e411e70) )
	ROM_LOAD32_BYTE( "5-1_sp_525.ic25", 0x040000, 0x010000, CRC(46bde779) SHA1(b4e1dd1952276c2d2a8f3c150d1ba2a1c2b738b7) ) // 11xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "5-1_sp_519.ic19", 0x040001, 0x010000, CRC(69cc9e06) SHA1(85e1495d01e6986f9cd88d6cdbef194c623be111) ) // 11xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "5-1_sp_513.ic13", 0x040002, 0x010000, CRC(659d206f) SHA1(fc8e9ea2d45df83509de3986763cbfc0d4745983) ) // has some garbage in the blank space of the paired ROMs
	ROM_LOAD32_BYTE( "5-1_sp_504.ic4",  0x040003, 0x010000, CRC(b6806390) SHA1(d95247bcd90bd7b7be355c267f023c19a9d60f66) ) // 11xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100, "prom", ROMREGION_ERASEFF )
	ROM_LOAD( "51-3_502_82s129.ic10",      0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // common PROM found on all? Modular System sets?

	ROM_REGION( 0x100, "protgal", 0 ) // all read protected
	ROM_LOAD( "5-1_5150_gal16v8as.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "5-1_5250_gal16v8as.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7150_gal20v8as.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7250_gal20v8as.ic54", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7350_gal16v8as.ic5", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7450_gal16v8as.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7550_gal16v8as.ic59", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7650_gal20v8as.ic44", 0, 1, NO_DUMP )
	ROM_LOAD( "51-3_503_gal16v8.ic46", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu1_6-1_605_gal16v8as.ic13", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu1_6-1_650_gal16v8as.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu2_c_c50_gal16v8as.ic10", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9-2_9159_gal16v8as.ic42", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9-2_9250_gal20v8as.ic18", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9-2_9359_gal16v8as.ic10", 0, 1, NO_DUMP )
ROM_END

GAME( 1991, splashms,  0,  splashms,  splashms,  splashms_state, empty_init, ROT0, "Gaelco", "Splash (Modular System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
