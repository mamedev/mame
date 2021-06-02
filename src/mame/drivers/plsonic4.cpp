// license:BSD-3-Clause
// copyright-holders:
/*
Play Sonic 4 by SegaSA / Sonic

This is a multi-game system. Up to 4 JAMMA PCBs can be connected and the player can decide which game to play.
The system offers digital counters for time of play and credits (configurable via dips), and game statistics.

The PCB doesn't seem to have any markings.
Main components are:
Z8400AB1 main CPU
3x 6116 RAMs
8-dip bank
4-dip bank
8 MHz XTAL (near main CPU)
20 MHz XTAL
lots of TTL
4x digital counters
lots of wires

There's a very small riser PCB marked 1B-2001-241 with a couple of TTL  and a slightly bigger one marked 1B-2001-238 with 3 TTL.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class plsonic4_state : public driver_device
{
public:
	plsonic4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram")
	{ }

	void plsonic4(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_tilemap;

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map);
	void io_map(address_map &map);
};


void plsonic4_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(plsonic4_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

TILE_GET_INFO_MEMBER(plsonic4_state::tile_info) // TODO: this is the bare minimum to see what's going on
{
	int code = m_videoram[tile_index * 2];

	//uint8_t color = ; //TODO

	tileinfo.set(0, code, 0, 0);
}

uint32_t plsonic4_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->mark_all_dirty();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void plsonic4_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x97ff).ram().share(m_videoram); // TODO: really all this range?
	map(0x9800, 0x99ff).ram(); // ??
}

void plsonic4_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	// map(0x00, 0x1f).w(); // digital counters?
	// map(0x00, 0x03).r(); // dips? coins? inputs? coin counters?
}


static INPUT_PORTS_START( plsonic4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") // 4 dip bank
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static GFXDECODE_START( gfx_plsonic4 )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x3_planar, 0, 16 ) // TODO: check this
GFXDECODE_END


void plsonic4_state::plsonic4(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &plsonic4_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &plsonic4_state::io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: this is just copy-pasted, needs to be fixed
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32 * 8, 32 * 8);
	screen.set_visarea(0 * 8, 32 * 8 - 1, 2 * 8, 30 * 8 - 1);
	screen.set_screen_update(FUNC(plsonic4_state::screen_update));

	PALETTE(config, "palette").set_entries(16);
	GFXDECODE(config, "gfxdecode", "palette", gfx_plsonic4);
}


ROM_START( plsonic4 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "segasa_m-12_play_sonic_4_1.bin", 0x0000, 0x8000, CRC(f7fb2259) SHA1(4525ad6c38b12e5abf6f57ed16963a4ce48f3c5d) ) // second half is almost empty

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "segasa_m-12_play_sonic_4_2.bin", 0x0000, 0x2000, CRC(58b2b6a0) SHA1(6271f83a0c7858add286e4faaf5999916debcb70) )
	ROM_LOAD( "segasa_m-12_play_sonic_4_3.bin", 0x2000, 0x2000, CRC(d124045b) SHA1(a6e258582a80b411e718df87927a240ff9c59b2d) )
	ROM_LOAD( "segasa_m-12_play_sonic_4_4.bin", 0x4000, 0x2000, CRC(3db5dd0a) SHA1(c9c17a5c696f2ded8362fab2658913cca630665d) )
ROM_END

} // Anonymous namespace


GAME( 1991, plsonic4, 0, plsonic4, plsonic4, plsonic4_state, empty_init, ROT0, "SegaSA / Sonic", "Play Sonic 4", MACHINE_IS_SKELETON )
