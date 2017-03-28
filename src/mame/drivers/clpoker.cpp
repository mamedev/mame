// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista

/*
Poker game (unknown name) by Chain Leisure
Main CPU ROM contains the following string:
"CLPOK GAME designed by FULL-LIFE  at 02-01-1994"


PCB is marked Chain Leisure CL-001

- 1x Z0840004PSC Z80 CPU
- 1x 12.000 X-TAL
- 1x AY38910A/P sound chip
- 2x M5L8255AP-5
- 1x HM86171 RAMDAC
- 1x GM76C88L-15 SRAM (8,192 x 8 Bit)
- 1x MACH110-20JC CMOS
- 2x ATV2500H PLDs
- 1x PLSI1024-60LJ CPLD
- 1x GAL20V8A-15LNC
- 1x bank of 8 dip-switches
*/

#include "emu.h"
#include "screen.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"

class clpoker_state : public driver_device
{
public:
	clpoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, clpoker_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, clpoker_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( clpoker )
INPUT_PORTS_END

uint32_t clpoker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( clpoker, clpoker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz / 3) // Z0840004PSC, divider not verified
	MCFG_CPU_PROGRAM_MAP(prg_map)
	MCFG_CPU_IO_MAP(io_map)

	// 2x M5L8255AP-5

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // wrong
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))  // wrong
	MCFG_SCREEN_SIZE(64*8, 32*8) // wrong
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1) // wrong
	MCFG_SCREEN_UPDATE_DRIVER(clpoker_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x1000)

	/* sound hardware */
	// AY38910A/P
MACHINE_CONFIG_END


ROM_START( clpoker )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "5.u7", 0x00000, 0x10000, CRC(96b07104) SHA1(24ec1e44795add0db6215a7687ac2fd3b636980b) ) // 27512, 2nd half empty?

	ROM_REGION(0x80000, "gfx1", 0)
	ROM_LOAD( "1.u1", 0x00000, 0x20000, CRC(d1b5a3f1) SHA1(5a08be220b81d9502f1ed61966916384925ba569) ) // 27C010
	ROM_LOAD( "2.u2", 0x20000, 0x20000, CRC(00abb6b2) SHA1(3123c2e18d987895cb1d3359bf2765343289037b) ) // 27C010
	ROM_LOAD( "3.u3", 0x40000, 0x20000, CRC(fcccef5a) SHA1(a0bdba24a6a9ca8aa8b7fdfee10ace3cb17600b4) ) // 27C010
	ROM_LOAD( "4.u4", 0x60000, 0x20000, CRC(be707d36) SHA1(b1cb9dc387a54d895cfaedfbc015598151ddab38) ) // 27C010

	ROM_REGION(0x1000, "pld", 0)
	ROM_LOAD( "plsi1024-60lj.pl1", 0x00, 0x200,  NO_DUMP )
	ROM_LOAD( "atv2500h.pl2",      0x00, 0x200,  NO_DUMP )
	ROM_LOAD( "atv2500h.pl3",      0x00, 0x200,  NO_DUMP )
	ROM_LOAD( "mach110-20jc.pl4",  0x00, 0x200,  NO_DUMP )
	ROM_LOAD( "gal20v8a.pl5",      0x00, 0x157,  NO_DUMP )
ROM_END


GAME( 1994, clpoker,  0, clpoker, clpoker, driver_device, 0, ROT0, "Chain Leisure", "Unknown Poker Game", MACHINE_IS_SKELETON ) // Year taken from string in main CPU ROM
