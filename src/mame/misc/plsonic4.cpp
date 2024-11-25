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
#include "machine/nvram.h"
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
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
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
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8fff).ram().share("nvram"); // TODO: verify size
	map(0x9000, 0x97ff).ram().share(m_videoram); // TODO: really all this range?
	map(0x9800, 0x99ff).ram(); // ??
}

void plsonic4_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	// map(0x00, 0x1f).nopw(); // digital counters? lamps?
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("DSW0");
	map(0x03, 0x03).portr("DSW1");
}


static INPUT_PORTS_START( plsonic4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("DSW0") // coins + 4 dip bank
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPSETTING(    0x20, "2:10" )
	PORT_DIPSETTING(    0x10, "2:20" )
	PORT_DIPSETTING(    0x30, "2:30" )
	PORT_DIPSETTING(    0x80, "2:40" )
	PORT_DIPSETTING(    0xa0, "2:50" )
	PORT_DIPSETTING(    0x90, "3:00" )
	PORT_DIPSETTING(    0xb0, "3:10" )
	PORT_DIPSETTING(    0x40, "3:30" )
	PORT_DIPSETTING(    0x60, "3:40" )
	PORT_DIPSETTING(    0x50, "4:00" )
	PORT_DIPSETTING(    0x70, "4:10" )
	PORT_DIPSETTING(    0xc0, "4:30" )
	PORT_DIPSETTING(    0xe0, "5:00" )
	PORT_DIPSETTING(    0xd0, "5:30" )
	PORT_DIPSETTING(    0xf0, "6:00" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0f, "Disabled" ) // 'inhibido'
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xf0, "Disabled" ) // 'inhibido'
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

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

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
