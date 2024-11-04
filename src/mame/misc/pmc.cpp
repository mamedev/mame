// license:BSD-3-Clause
// copyright-holders:

/*
PMC Pave 2000 PCB, MB104 Rev.2

2 crystals on PCB

7.864 crystal at edge
5.185 near clock generator chip

4 x 4702 EPROM
1 x C4289 memory interface
1 x C4201 clock generator
1 x C4040 CPU
1 x P4002 ram
1 x N8T96B bus driver
5 x 82S126 proms

Two daughter boards, 1 has on/off switch and 2 hex thumbwheels (0-F) and various TTL

There are 4 x NE555V chips. All connect via resistors to edge connector (paddles / joystick ?)
*/


#include "emu.h"

#include "cpu/mcs40/mcs40.h"

#include "screen.h"
#include "speaker.h"

namespace {

class pmc_state : public driver_device
{
public:
	pmc_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	{
	}

	void pmc(machine_config &config);

private:
	void rom_map(address_map &map) ATTR_COLD;
	void rom_ports_map(address_map &map) ATTR_COLD;
	void ram_map(address_map &map) ATTR_COLD;
	void ram_status_map(address_map &map) ATTR_COLD;
};


void pmc_state::rom_map(address_map &map)
{
	map(0x000, 0x3ff).rom().region("maincpu", 0x000);
}

void pmc_state::rom_ports_map(address_map &map)
{
	// TODO
}

void pmc_state::ram_map(address_map &map)
{
	map(0x000, 0x03f).ram();
}

void pmc_state::ram_status_map(address_map &map)
{
	map(0x000, 0x00f).ram();
}

void pmc_state::pmc(machine_config &config)
{
	// basic machine hardware
	i4040_cpu_device &cpu(I4040(config, "maincpu", 5.185_MHz_XTAL / 7)); // P4201A divides the incoming clock by seven to get the multi-phase clock
	cpu.set_rom_map(&pmc_state::rom_map);
	cpu.set_rom_ports_map(&pmc_state::rom_ports_map);
	cpu.set_ram_memory_map(&pmc_state::ram_map);
	cpu.set_ram_status_map(&pmc_state::ram_status_map);

	// video hardware
	// SCREEN(config, "screen", SCREEN_TYPE_RASTER); // TODO

	// sound hardware
	// TODO: netlist
}


ROM_START( unkpmc )
	ROM_REGION( 0x400, "maincpu", 0 )
	ROM_LOAD( "4", 0x000, 0x0100, CRC(7bafe4ad) SHA1(afb02aebf2ff9e733368545c0c0100bdcaa5c89b) )
	ROM_LOAD( "3", 0x100, 0x0100, CRC(727f2186) SHA1(8c1dcbf3099010f510400db03946d7aba813cf97) )
	ROM_LOAD( "2", 0x200, 0x0100, CRC(9e6a0570) SHA1(fbf6ce4066121019d0db0de0c75fb5869853c613) )
	ROM_LOAD( "1", 0x300, 0x0100, CRC(396fc572) SHA1(29b39d1119b28f6c75378f855129e132a5cb2370) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "5",  0x000, 0x0100, CRC(4023885f) SHA1(af94454583af10938746caf7bbe326081d21241d) )
	ROM_LOAD( "7",  0x100, 0x0100, CRC(0a5174d7) SHA1(e4d8b5227adfcf49e1cb9b626ece28a3844ddb17) )
	ROM_LOAD( "8",  0x200, 0x0100, CRC(c5f1125a) SHA1(7686feadb7c6047fafedd070c8578e7f4736e15f) )
	ROM_LOAD( "9",  0x300, 0x0100, CRC(d9adaa48) SHA1(ddd76e1469e63826a199d12a8cbff56020c9dec3) )
	ROM_LOAD( "10", 0x400, 0x0100, CRC(df9230a1) SHA1(66b4462699837aff09dbc8de80257c5737b4d91a) )
ROM_END

} // Anonymous namespace


GAME( 1975?, unkpmc, 0, pmc, 0, pmc_state, empty_init, ROT0, "PMC", "unknown PMC game", MACHINE_IS_SKELETON ) // might be Aztec Princess
