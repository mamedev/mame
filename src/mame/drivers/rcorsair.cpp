// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Red Corsair */

/* skeleton driver */
/* This driver is not being worked on by the original author.
   Somebody will probably need to do extensive research on the
   PCB to establish what the custom block actually contains,
   and how to extract it  */


/*  Readme notes

Red Corsair EPROM dump (dumped by Phil Morris) - notes follow:

http://www.morris0.fsnet.co.uk

The game itself involves you guiding a pirate around various obstacles and
picking up treasure. It's a top-down viewpoint.

Relevant part of the display says:  (c) Nakasawa 1984

There is one empty (EPROM?) socket, the rest are populated with 4 x
2764's and 1 x 2763

There are 2 x AY-3-8910 sound chips, 1 x 8255, 1 x HD46505SP video
chip (40-pin), and an 8085 CPU.

The large metal enclosure in the middle is connected with 2 x 20-way
SILs and even when removed is resin coated. I'd guess this contains the main CPU
and no doubt some sort of protection (I don't really want to pull this apart
as it currently seems to be one of a kind). So I guess that limits the
possibilities of getting the code running under MAME?

The EPROM dumps are named by the name on the labels, an underscore, then the
location on the PCB by column (number) then row (letter).

The two PROMs are both AM27S21DC (ie 256 x 4)

*/

/*

Driver Notes:

Can't do much with this, I'm pretty sure the game code is in the custom block
so even the Main CPU is unknown, assuming the 8085 is the sound CPU

Notes added 2014-09-10:
- Rom in "user1" contains Z80 code (not 8085 code), and fits in the memory map at 4000-5FFF.
   It contains subroutines meant to be called from elsewhere.
- Rom in "maincpu" is not 8085 code (unless it is scrambled)


*/


#include "emu.h"
#include "cpu/i8085/i8085.h"


class rcorsair_state : public driver_device
{
public:
	rcorsair_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
};


static ADDRESS_MAP_START( rcorsair_map, AS_PROGRAM, 8, rcorsair_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( inports )
	PORT_START("IN0")
	PORT_DIPNAME(   0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( rcorsair )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

void rcorsair_state::video_start()
{
}

UINT32 rcorsair_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( rcorsair, rcorsair_state )

	/* Main CPU is probably inside Custom Block with
	   program code, unknown type */

	MCFG_CPU_ADD("maincpu", I8085A,8000000)      /* Sound CPU? */
	MCFG_CPU_PROGRAM_MAP(rcorsair_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rcorsair_state,  irq0_line_hold)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(rcorsair_state, screen_update)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rcorsair)
	MCFG_PALETTE_ADD("palette", 0x100)
MACHINE_CONFIG_END

ROM_START( rcorsair )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "rcs_6d.bin", 0x00000, 0x2000, CRC(b7f34f91) SHA1(16d5ed6a60db09f04727be8500c1c8c869281a8a) ) // sound code? (or part of the game code?)

	ROM_REGION( 0x2000, "cpu1", 0 )
	ROM_LOAD( "custom_block", 0x00000, 0x2000, NO_DUMP ) // assume it is CPU, could be wrong, it needs investigating anyway

	ROM_REGION( 0x6000, "gfx1", 0 ) /* there looks to be a slight scramble per tile, probably simle address xor */
	ROM_LOAD( "rcd2_2b.bin", 0x0000, 0x2000, CRC(d52b39f1) SHA1(20ce812fb4a9157d7c1d45902645695f0dd84add) )
	ROM_LOAD( "rcd1_2c.bin", 0x2000, 0x2000, CRC(9ec5dd51) SHA1(84939799f64d9d3e9a67b51046dd0c3403904d97) )
	ROM_LOAD( "rcd0_2d.bin", 0x4000, 0x2000, CRC(b86fe547) SHA1(30dc51f65d2bd807d2498829087ba1a8eaa2e146) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "red47_2r.bin", 0x00000, 0x2000, CRC(25ae59c2) SHA1(ee68faeba81e1c426184532bd736be574a08f7d4) ) // ? sound code/data? game data?

	ROM_REGION( 0x40000, "proms", 0 )
	ROM_LOAD( "prom_3d.bin", 0x00000, 0x100, CRC(fd8bc85b) SHA1(79324a6cecea652bc920ec762e7a30044003ed3f) ) // ?
	ROM_LOAD( "prom_3c.bin", 0x00000, 0x100, CRC(edca1d4a) SHA1(a5ff659cffcd09cc161960da8f5cdd234e0db92c) ) // ?
ROM_END


GAME( 1984, rcorsair,  0,    rcorsair, inports, driver_device, 0, ROT90, "Nakasawa", "Red Corsair", MACHINE_IS_SKELETON )
