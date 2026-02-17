// license:BSD-3-Clause
// copyright-holders:

/*
Dharma Tokyo Z180-based games

Main components:

HD647180 (undumped internal ROM)
12 MHz XTAL
HM6264P-15 (with a second unpopulated socket - possibly removed)
4x scratched off chip (I8255?)
YM2203C
3x bank of 8 switches

TODO:
* find a way to dump internal ROM;
* everything else.
*/

#include "emu.h"

#include "cpu/z180/hd647180x.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dharma_state : public driver_device
{
public:
	dharma_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void lbingo(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


uint32_t dharma_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void dharma_state::program_map(address_map &map)
{
	map(0x00000, 0x03fff).rom();
}

void dharma_state::io_map(address_map &map)
{
}


static INPUT_PORTS_START( lbingo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )
INPUT_PORTS_END


static GFXDECODE_START( gfx_lbingo )
	GFXDECODE_ENTRY( "tiles1", 0, gfx_8x8x8_raw, 0, 16 )
	GFXDECODE_ENTRY( "tiles2", 0, gfx_8x8x8_raw, 0, 16 ) // TODO
GFXDECODE_END


void dharma_state::lbingo(machine_config &config)
{
	HD647180X(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dharma_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &dharma_state::io_map);

	// TODO: everything
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(dharma_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_lbingo);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym", 12_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 1.00); // divider not verified
}


ROM_START( lbingo )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_rom.u28", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD( "lb1.u18", 0x00000, 0x20000, CRC(259d2519) SHA1(447fccbc16a48995513a10ba48ed838fc5edbe87) )

	ROM_REGION( 0x80000, "tiles2", 0 ) // TODO: possibly in reverse order
	ROM_LOAD( "lb2.u34", 0x00000, 0x20000, CRC(08e5d09c) SHA1(b3f6c3872cf2ac868093d5e3590c1c84ce4d5f87) )
	ROM_LOAD( "lb3.u33", 0x20000, 0x20000, CRC(d2ac0618) SHA1(705ec4328640d1b3f111c25b9b504e3e818c91a1) )
	ROM_LOAD( "lb4.u32", 0x40000, 0x20000, CRC(e5b8aac3) SHA1(4419717f563989bd1fd7250a1bcef838d40e15e1) )
	ROM_LOAD( "lb5.u31", 0x60000, 0x20000, CRC(da8eeb5f) SHA1(2d6b37ae8077376ae40e9d2ea2564142c0d68408) )
ROM_END

} // anonymous namespace

// title and year taken from GFX assets (spelling is accurate)
GAME( 1996, lbingo, 0, lbingo, lbingo, dharma_state, empty_init, ROT0, "Dharma Tokyo", "Luckey Bingo", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
