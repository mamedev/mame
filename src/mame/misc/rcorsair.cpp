// license:BSD-3-Clause
// copyright-holders:David Haywood
// Red Corsair

// skeleton driver
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

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class rcorsair_state : public driver_device
{
public:
	rcorsair_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_subcpu(*this, "subcpu")
	{ }

	void rcorsair(machine_config &config);

protected:
	// driver_device overrides
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void rcorsair_main_map(address_map &map) ATTR_COLD;
	void rcorsair_sub_io_map(address_map &map) ATTR_COLD;
	void rcorsair_sub_map(address_map &map) ATTR_COLD;
};


void rcorsair_state::rcorsair_main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0xa000, 0xa03f).ram();
}

void rcorsair_state::rcorsair_sub_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void rcorsair_state::rcorsair_sub_io_map(address_map &map)
{
}

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
	{ 2, 3, 0, 1, 6, 7, 4, 5 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_rcorsair )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

void rcorsair_state::video_start()
{
}

uint32_t rcorsair_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void rcorsair_state::rcorsair(machine_config &config)
{
	// Main CPU is probably inside Custom Block with program code, unknown type

	Z80(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rcorsair_state::rcorsair_main_map);
	//m_maincpu->set_vblank_int("screen", FUNC(rcorsair_state::irq0_line_hold));

	I8035(config, m_subcpu, 8000000);
	m_subcpu->set_addrmap(AS_PROGRAM, &rcorsair_state::rcorsair_sub_map);
	m_subcpu->set_addrmap(AS_IO, &rcorsair_state::rcorsair_sub_io_map);

	I8255(config, "ppi");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(rcorsair_state::screen_update));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_rcorsair);
	PALETTE(config, "palette").set_entries(0x100);

	HD6845S(config, "crtc", 8000000 / 8).set_screen("screen");

	SPEAKER(config, "speaker").front_center();

	AY8910(config, "ay1", 8000000 / 8).add_route(ALL_OUTPUTS, "speaker", 0.5);

	AY8910(config, "ay2", 8000000 / 8).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

ROM_START( rcorsair )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "custom_block", 0x0000, 0x4000, NO_DUMP ) // assume it is CPU, could be wrong, it needs investigating anyway
	ROM_FILL(                 0x0000, 0x4000, 0 )
	ROM_LOAD( "red47_2r.bin", 0x4000, 0x2000, CRC(25ae59c2) SHA1(ee68faeba81e1c426184532bd736be574a08f7d4) ) // ? sound code/data? game data?

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "rcs_6d.bin", 0x00000, 0x2000, CRC(b7f34f91) SHA1(16d5ed6a60db09f04727be8500c1c8c869281a8a) ) // sound code? (or part of the game code?)

	ROM_REGION( 0x6000, "gfx1", 0 ) // there looks to be a slight scramble per tile, probably simple address XOR
	ROM_LOAD( "rcd2_2b.bin", 0x0000, 0x2000, CRC(d52b39f1) SHA1(20ce812fb4a9157d7c1d45902645695f0dd84add) )
	ROM_LOAD( "rcd1_2c.bin", 0x2000, 0x2000, CRC(9ec5dd51) SHA1(84939799f64d9d3e9a67b51046dd0c3403904d97) )
	ROM_LOAD( "rcd0_2d.bin", 0x4000, 0x2000, CRC(b86fe547) SHA1(30dc51f65d2bd807d2498829087ba1a8eaa2e146) )

	ROM_REGION( 0x40000, "proms", 0 )
	ROM_LOAD( "prom_3d.bin", 0x00000, 0x100, CRC(fd8bc85b) SHA1(79324a6cecea652bc920ec762e7a30044003ed3f) ) // ?
	ROM_LOAD( "prom_3c.bin", 0x00000, 0x100, CRC(edca1d4a) SHA1(a5ff659cffcd09cc161960da8f5cdd234e0db92c) ) // ?
ROM_END

} // anonymous namespace


GAME( 1984, rcorsair,  0,    rcorsair, inports, rcorsair_state, empty_init, ROT90, "Nakasawa", "Red Corsair", MACHINE_IS_SKELETON )
