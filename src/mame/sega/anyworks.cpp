// license:BSD-3-Clause
// copyright-holders:

/*
unknown AnyWorks medal game system bios / boot code (?)
It may be a cooperation with Sega, as some of the codes below seem to imply.
A 'Sega' string is also found in the flash ROM at 0x28b388f.
A '2010.04.16' is found at 0x28b34e8.
The flash ROM contains generic medal-related strings and sound effects.

PCB has the following codes: 94V-0, AW-128M-65, 838-15023, 834-14963 and AW10Z00100108

The recognizable components are:
Peak Microtech Eagle PKM32AG-Q SoC with 32bit EISC(AE32000C) Processor Core
14.3181 MHz XTAL
65.000 MHz XTAL
SD card slot
*/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class anyworks_state : public driver_device
{
public:
	anyworks_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void unkanyw(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t anyworks_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void anyworks_state::program_map(address_map &map)
{
	map(0x00000000, 0x083fffff).rom(); // map everything here for now
}


static INPUT_PORTS_START( unkanyw )
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


void anyworks_state::unkanyw(machine_config &config)
{
	ARM9(config, m_maincpu, 14'318'100); // placeholder for unemulated SoC
	m_maincpu->set_addrmap(AS_PROGRAM, &anyworks_state::program_map);

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(anyworks_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100); // wrong

	SPEAKER(config, "mono").front_center();
}


ROM_START( unkanyw )
	ROM_REGION(0x8400000, "maincpu", 0)
	ROM_LOAD( "u13", 0x0000000, 0x8400000, CRC(fe65f34a) SHA1(0eda682e3f3b5bc8dadf3c536fbe23c1015fee03) )
ROM_END

} // anonymous namespace


GAME( 2010, unkanyw, 0, unkanyw, unkanyw, anyworks_state, empty_init, ROT0, "AnyWorks / Sega", "unknown AnyWorks / Sega medal game BIOS", MACHINE_IS_SKELETON )
