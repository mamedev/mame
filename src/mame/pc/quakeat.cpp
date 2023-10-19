// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*

 Quake Arcade Tournament

 Only the HDD is dumped.  The HDD is stickered 'Release Beta 2'

 We've also seen CDs of this for sale, so maybe there should be a CD too, for the music?

TODO:
- In pcipc throws "E0409 -- Security key not found." in glquake.exe when it starts running.

===================================================================================================
 -- set info

Quake Arcade Tournament by Lazer-Tron

PC running Windows 95 with a Dongle on the parallel port

Created .chd with version 0.125

It found the following disk paramaters...

Input offset    511
Cylinders   263
Heads       255
Sectors     63
Byte/Sector 512
Sectors/Hunk    8
Logical size    2,163,248,864

The "backup" directory on hard disk was created by the dumper.

 -- Hardware info found on the following web pages:
http://web.archive.org/web/20070810060806/http://www.wave-report.com/archives/1998/98170702.htm
http://www.thedodgegarage.com/3dfx/q3d_quicksilver.htm
http://quakearcadetournament.blogspot.com/
http://web.archive.org/web/20001001045148/http://www.quantum3d.com:80/press%20releases/4-20-98.html
https://www.quaddicted.com/webarchive/www.quaddicted.com/quake-nostalgia/quake-arcade-tournament-edition/

Quantum3D Heavy Metal HM233G (part of Quantum3D's Quicksilver family)
- NLX form factor system that is based on the Intel 440LX chipset
- Intel NX440LX motherboard
    - Intel 82440LX AGPset (82443LX Northbridge / 82371AB PIIX4 PCI-ISA Southbridge)
    - SMC FDC37C677 I/O
    - Yamaha OPL3-SA3 (YMF715) Audio codec (16-bit per sample 3D audio)
    - Intel Pro 10/100 PCI Ethernet NIC
    - (Optional) Cirrus CL-GD5465 AGP Graphics Controller
    - Intel/Phoenix BIOS
- Intel Pentium II 233 233MHz CPU processor with 512KB of L2 cache
- (1) 32MB PC66 66 MHz SDRAM 168-pin DIMM
- Microsoft Windows 95 OSR2.5
- shock-mounted 3.1GB Ultra DMA-33 EIDE hard drive
- 12-24x CD-ROM drive
- 1.44 MB floppy drive
- Quantum3D Obsidian2 90-4440 AGP AGPTV Voodoo2-based realtime 3D graphics accelerator
      (a professional version of the Quantum3D Obsidian2 S-12 AGPTV)
- Companion PCI 2D/VGA: Quantum3D Ventana MGV-PCI (Alliance Semiconductor ProMotion aT25)
      or Quantum3D Ventana "MGV Rush" (custom Quantum3D Ventana 50 Voodoo Rush with 3D-disabled
      2D-only BIOS and no TV-out, only using the Alliance Semiconductor ProMotion aT25)
- Quantum3D GCI-2 (Game Control Interface II) I/O board - designed to interface coin-op and
      industrial input/output control devices to a PC. Fits in either PCI or ISA bus slot for
      mechanical attachment only. Communications between the GCI and the PC are via a standard
      RS-232 serial interface, using a 14-byte packet protocol. Power is provided by a 4-pin
      Molex style disk driver power connector.

Note: Quantum3D Quicksilver QS233G configuration seem very similar to the HM233G, with the only
    exceptions being that the Quantum3D Obsidian2 90-4440 is replaced with the earlier Quantum3D
    Obsidian 100SB-4440V Voodoo Graphics realtime 3D graphics accelerator with 2D Alliance
    Semiconductor ProMotion aT25 MGV 2000 daughter card, and the Quantum3D GCI-2 might be replaced
    with an earlier Quantum3D GCI.

Dongle: Rainbow Technologies parallel-port security dongle (at least 1024 bytes)

HDD image contains remnants of an Actua Soccer Arcade installation.

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class quakeat_state : public driver_device
{
public:
	quakeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void quake(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void quake_map(address_map &map);
};


void quakeat_state::quake_map(address_map &map)
{
	map(0x000e0000, 0x000fffff).rom().region("pc_bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("pc_bios", 0);

}


static INPUT_PORTS_START( quake )
INPUT_PORTS_END


void quakeat_state::quake(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 233'000'000); /* Pentium II, 233MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &quakeat_state::quake_map);
//	m_maincpu->set_disable();

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(quake)
	// 4N4XL0X0.86A.0011.P05
	ROM_REGION32_LE(0x20000, "pc_bios", 0)  /* motherboard bios */
	// TODO: compressed
//	ROM_LOAD("p05-0011.bio", 0x000000, 0x10000, NO_DUMP )
//	ROM_CONTINUE( 0x1ffff-0xa0, 0xa0 )
	ROM_LOAD("quakearcadetournament.pcbios", 0x000000, 0x20000, NO_DUMP )

	// Hitachi DK237A-21 A/A0A0, IDE/ATA 2.5" 2.1GB 4000 RPM
	// WS03131880
	DISK_REGION( "disks" )
	// wrong chs 263,255,63
//	DISK_IMAGE( "quakeat", 0, BAD_DUMP SHA1(c44695b9d521273c9d3c0e18c88f0dca0185bd7b) )
	// regenerated from above, with -chs 4200,16,63 as per reported HDD label
	DISK_IMAGE( "quakeat", 0, BAD_DUMP SHA1(9a422ad342aeddd447514d0287efde49e3de5fa8) )
ROM_END

} // anonymous namespace


GAME( 1998, quake,  0,   quake, quake, quakeat_state, empty_init, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", MACHINE_IS_SKELETON )
// Actua Soccer Arcade
