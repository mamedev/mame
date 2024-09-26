// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

TODO:
- SIGABRT in pcipc/pciagp trying to execute ppm.exe,
  in shutms11 will draw "Parallel Port Manager v4.0" then fail on device check.
\- "Testing I/O board connection"
   bp 100027e2, edit $25dd11 = 0x02 (board identifier?)
\- "Testing I/O board communications"
   bp 10002942 (checks acknowledge from control bit 6 @ $37a, for 0xe0-0xe7. Buffers at $25dd48-4f)
   Expected ack values: 40 40 00 00 40 00 40 00
   bp 10002d57,1,{ebx=8;g} for a quick workaround
\- "Testing I/O board input bits" / "BAD 0 1 2 3     7"
   Goes ahead in checking each port r/w, TBD
\- Fails win98 PS/2 PnP afterwards, which isn't supposed to be connected in the first place.
- Extract "Guard.zip" and understand what is for;

===================================================================================================

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
C:\Windows has driver installs for:
- a Sound Blaster AudioPCI 128
- an ATI All-In-Wonder / All-In-Wonder Pro (with leftover "SYSTEM.I~I" footprint with
  "display.drv=ATI Rage IIC AGP (Português)").

**************************************************************************************************/

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

	void neomania_map(address_map &map) ATTR_COLD;
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
	DISK_REGION( "ide:0:hdd" ) // From a Norton Ghost recovery image
	DISK_IMAGE( "neomania", 0, SHA1(4338be8214ca5a9aa6808a94d73e0820a4d34b98) )
ROM_END

} // Anonymous namespace

GAME( 2003, neomania, 0, neomania, neomania, neomania_state, empty_init, ROT0, "bootleg (Hyper M.A.R.)", "Neo Mania (Portugal)", MACHINE_IS_SKELETON )
