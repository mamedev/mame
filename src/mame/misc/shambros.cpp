// license:BSD-3-Clause
// copyright-holders:

/*
Kato's STV01 MainPCB Rev. C


Only known game: Shamisen Brothers Vol 1 (2003)
The game was also ported for Namco System 10

The dumper only had the CD-ROM.
A low quality picture of the PCB found on the internet shows:
- M68K-based processor
- program ROM
- multiple flash chips
- 3 FPGAs
- Mitsumi CD drive

TODO: only a placeholder for the CD dump for now
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class shambros_state : public driver_device
{
public:
	shambros_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{}

	void shambros(machine_config &config) ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


uint32_t shambros_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static INPUT_PORTS_START( shambros )
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

	// no dips on PCB
INPUT_PORTS_END


void shambros_state::shambros(machine_config &config)
{
	M68000(config, "maincpu", 12'000'000); // exact type not known, XTAL unreadable

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(shambros_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100); // wrong

	SPEAKER(config, "mono").front_center();
}


ROM_START( shambros )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "g112 v1.01.prg", 0x00000, 0x20000, NO_DUMP ) // actual size unknown

	// numerous undumped flash ROMs

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "sb01-100", 0, SHA1(abd1d61871bcb4635acc691e35ec386823763ba2) )
ROM_END

} // anonymous namespace


GAME( 2003, shambros, 0, shambros, shambros, shambros_state, empty_init, ROT0, "Kato's", "Shamisen Brothers Vol 1", MACHINE_IS_SKELETON )
