// license:BSD-3-Clause
// copyright-holders:
/*
Neo Mania:
 The Portuguese (Vila Nova de Gaia) company "Hyper M.A.R." created this machine on 2002 with 40 games,
 and updated it on 2003 increasing the number of games up to 48. There was a latest newer version
 where they added "Strikers 1945" and "Prehistoric Isle 2", reaching 50 games.
 There are Spanish and Portuguese localizations.
 The hardware is a PC with Windows 98 (exact hardware not specified) and a Neo·Geo emulator, with a
 small PCB for converting VGA + Parallel port (inputs) + sound (with volume knob) to JAMMA (named
 "NEO MANIA ADAPTER BOARD").
The "NEO MANIA ADAPTER BOARD" contains:
   3 x Blocks of jumpers to enable or disable features:
    JMP1 (two positions) - With or without Coin Dist.
    JMP2 (two positions)  - With or without Audio Amplifier
    JMP3 (three positions) - Unknown function
   2 x Coin acceptors ports
   1 x Bank of 8 dipswitches (unknown function)

C:\Neomania folder contains ppm.exe, which is the driver for the parallel port device.
It also contains a password protected "Guard.zip", copy protection?
C:\Windows has driver installs for a PCI Sound Blaster and an ATI mach 64 / Bt829 derivative.


TODO:
- HDD image doesn't boot in neither shutms11 nor pcipc, mangled MBR boot record or geometry params (has -chs 3532,16,38 but WinImage reports back ~20 GB partition?);
- (With manually c&p files in a CHD that works) SIGABRT in pcipc trying to execute ppm.exe, in shutms11 will draw "Parallel Port Manager v4.0" then fail on device check;
- Extract "Guard.zip" and understand what is for;


*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class neomania_state : public driver_device
{
public:
	neomania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void neomania(machine_config &config);


private:
	required_device<cpu_device> m_maincpu;

	void neomania_map(address_map &map);
};


void neomania_state::neomania_map(address_map &map)
{
}

static INPUT_PORTS_START( neomania )
INPUT_PORTS_END


void neomania_state::neomania(machine_config &config)
{
	// Basic machine hardware
	// Neoemu.exe requires a processor with at least MMX features
	PENTIUM3(config, m_maincpu, 100'000'000); // Exact hardware not specified
	m_maincpu->set_addrmap(AS_PROGRAM, &neomania_state::neomania_map);
	m_maincpu->set_disable();

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************
  Game drivers
***************************************************************************/

ROM_START( neomania )

	// Different PC motherboards with different configurations.
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("pcbios.bin", 0x00000, 0x80000, NO_DUMP)

	// Portuguese version with 48 games, from 2003
	DISK_REGION( "ide:0:hdd:image" ) // From a Norton Ghost recovery image
	DISK_IMAGE( "neomania", 0, BAD_DUMP SHA1(4a865d1ed67901b98b37f94cfdd591fad38b404a) )
ROM_END

} // Anonymous namespace

GAME( 2003, neomania, 0, neomania, neomania, neomania_state, empty_init, ROT0, "bootleg (Hyper M.A.R.)", "Neo Mania (Portugal)", MACHINE_IS_SKELETON )
