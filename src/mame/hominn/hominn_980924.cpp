// license:BSD-3-Clause
// copyright-holders:

/*
Hom Inn 980924-1 PCB
JC-10011A

- square 100-pin chip with no markings
- rectangular 64-pin chip with no markings
- square 84-pin chip with no markings
- 16.000 MHz XTAL
- 12.000 MHz XTAL
- 4x HM6116P-3
- 2x LP6264D-70LL
- M5M82C255
- OKIM6295 or clone (markings unreadable, but ROM content reveals it)
- bank of 8 switches (with 2 unpopulated spaces for more banks)
*/


#include "emu.h"

#include "cpu/mcs51/i80c52.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hominn_980924_state : public driver_device
{
public:
	hominn_980924_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void qxjl(machine_config &config) ATTR_COLD;


private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t hominn_980924_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void hominn_980924_state::program_map(address_map &map)
{
}


static INPUT_PORTS_START( qxjl )
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

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x4_packed_msb, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_16x16x4_packed_lsb, 0, 16 )
GFXDECODE_END


void hominn_980924_state::qxjl(machine_config &config)
{
	// basic machine hardware
	I80C52(config, m_maincpu, 12_MHz_XTAL); // TODO: unknown CPU, XTAL could also be the 16 MHz one
	m_maincpu->set_addrmap(AS_PROGRAM, &hominn_980924_state::program_map);

	// 82C255 (actual chip on PCB) is equivalent to two 8255s
	I8255(config, "ppi0");

	I8255(config, "ppi1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(hominn_980924_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 16_MHz_XTAL / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // XTAL could also be the 12 MHz one, divider and pin 7 not verified
}


// 千禧接龙 (Qiānxǐ Jiēlóng)
ROM_START( qxjl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// internal to one of the 3 unidentified chips?

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "3.uz10", 0x00000, 0x20000, CRC(35cbe0cd) SHA1(f2c11d6e12097e281df6a6bd2ce35c15ef482377) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "5.ub9", 0x00000, 0x40000, CRC(f12e5c72) SHA1(e4d791bb623a10ee5041f8b52ba82b3e5bb7f5b7) )

	ROM_REGION( 0x50000, "unsorted", 0 )
	ROM_LOAD( "1.u17", 0x00000, 0x20000, CRC(50c0d1fb) SHA1(4d535c8e3032e651ed0d9b10206530fa5ddebf85) )
	ROM_LOAD( "2.u16", 0x20000, 0x20000, CRC(36eb936c) SHA1(348f842928177ab72ee62d93a9fb74e9d0cf5fb1) )
	ROM_LOAD( "4.ub4", 0x40000, 0x10000, CRC(7b44beed) SHA1(9cbdb5dc388665ded2c46d29e19ebcde194e7bc1) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "6.u31", 0x00000, 0x80000, CRC(fad9be9f) SHA1(d58a51b09560edffebe52ec22080a29767273ed3) )
ROM_END

} // anonymous namespace


GAME( 199?, qxjl, 0, qxjl, qxjl, hominn_980924_state, empty_init, ROT0, "Hom Inn", "Qianxi Jielong", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
