// license:BSD-3-Clause
// copyright-holders:

/*
Winner's Club - Mighty 8 Liner

Many games in the series, only one dumped for now.

Winner's Wheel - Magical Spot (2005)
https://www.youtube.com/watch?v=k3REoIx7m-I

Hardware manufactured by Sammy

main PCB (Sammy AM3AEE-02):
2x big Sammy customs with no other markings (1 believed to be an SH-4 core)
1x L14.318 XTAL
1x CY37256VP150 CPLD
2x 5212325FBPB60 128Mb SDRAMs
2x 48lC8M16A2 128Mb SDRAMs
1x TC554001 SRAM
1x TC58FVM6T2A flash memory (empty, possibly code gets uploaded from the PCMCIA card?)
1x YMZ770C-F

I/O PCB (Sammy AM3AEG-02):
1x TPD2005F High-Side Power Switch Array (8 Channels) for Motors, Solenoids, and Lamp Drivers
1x TPD2007F Low-Side Power Switch Array (8 Channels) for Motors, Solenoids, and Lamp Drivers
2x TD62084AF 8 Channel Darlington Sink Driver
many capacitors, resistors, etc

PCMCIA card sticker on the back: WWM2010-023.001 EA0-U-00146-1T (16MB) (TC58FVT641 x 2)
PCMCIA PCB (Sammy AM3AEJ-01):
2x TC58FVM6T2A flash memories
1x CY37032VP44-100AC CPLD (CY37064VP44 silkscreened on PCB but actually the former)
1x BR24C01FV-W
*/

#include "emu.h"

#include "cpu/sh/sh4.h"
#include "sound/ymz770.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class winclub_state : public driver_device
{
public:
	winclub_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void winclub(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
};

uint32_t winclub_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void winclub_state::prg_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom().mirror(0xe0000000).region("pcmcia", 0);
}

static INPUT_PORTS_START( winclub ) // no dips on PCB
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void winclub_state::winclub(machine_config &config)
{
	// basic machine hardware
	sh4_device &maincpu(SH4LE(config, "maincpu", 14'318'000)); // TODO: SH-4 based custom, unknown type and clock
	maincpu.set_addrmap(AS_PROGRAM, &winclub_state::prg_map);
	// maincpu.set_vblank_int("screen", FUNC(winclub_state::irq2_line_hold));

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(winclub_state::screen_update));

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz770_device &ymz770(YMZ770(config, "ymz", 16'384'000)); // internal clock?
	ymz770.add_route(0, "lspeaker", 1.0);
	ymz770.add_route(1, "rspeaker", 1.0);
}


ROM_START( wwmspot )
	ROM_REGION(0x800000, "maincpu", 0)
	ROM_LOAD( "tc58fvm6t2a.u41", 0x000000, 0x800000, CRC(1ad2bc45) SHA1(5fde1cce603e6566d20da811c9c8bcccb044d4ae) ) // empty. Code uploaded from PCMCIA? Or not used?

	ROM_REGION64_LE(0x1000000, "pcmcia", 0)
	ROM_LOAD( "tc58fvm6t2a.u4", 0x000000, 0x800000, CRC(f94e24b2) SHA1(4f6d4ba05da935cd6bd2323bd5950029caad436f) )
	ROM_LOAD( "tc58fvm6t2a.u3", 0x800000, 0x800000, CRC(3e115847) SHA1(09990946220c8f177200531e610a60488247c0a0) )

	ROM_REGION(0x400000, "ymz", ROMREGION_ERASE00) // is this uploaded too?

	ROM_REGION(0x80, "pmmcia_eeprom", 0)
	ROM_LOAD( "br24c01a.u2", 0x00, 0x80, CRC(222d9eec) SHA1(24a1cc415b0786695247caaca012bc51dda5349a) )
ROM_END

} // anonymous namespace


GAME( 2005, wwmspot, 0, winclub, winclub, winclub_state, empty_init, ROT0, "Sega", "Winner's Wheel - Magical Spot", MACHINE_IS_SKELETON ) // year taken from instructions on cabinet (see YouTube video)
