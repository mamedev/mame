// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*

 Quake Arcade Tournament

 Only the HDD is dumped.  The HDD is stickered 'Release Beta 2'

 We've also seen CDs of this for sale, so maybe there should be a CD too, for the music?

TODO:
can't be emulated without proper mb bios

 -- set info

Quake Arcade Tournament by Lazer-Tron

PC running Windows 95 with a Dongle on the parallel port

Created .chd with version 0.125

It found the following disk paramaters...

Input offset    511
Cyclinders  263
Heads       255
Sectors     63
Byte/Sector 512
Sectors/Hunk    8
Logical size    2,1163,248,864

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

===============================================================================
TODO:
    * Add BIOS dump (standard NX440LX motherboard)
    * Hook up PC hardware
    * Hook up the Quantum3D GCI-2 (details? ROMs?)
    * What's the dongle do?
===============================================================================
*/

#include "emu.h"

#include "pcshare.h"

#include "cpu/i386/i386.h"
#include "emupal.h"
#include "screen.h"


namespace {

class quakeat_state : public pcat_base_state
{
public:
	quakeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		{ }

	void quake(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_quake(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void quake_io(address_map &map);
	void quake_map(address_map &map);
};


void quakeat_state::video_start()
{
}

uint32_t quakeat_state::screen_update_quake(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void quakeat_state::quake_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("pc_bios", 0); /* BIOS */
}

void quakeat_state::quake_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e8, 0x00eb).noprw();
//  map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_device::read_cs0), FUNC(ide_controller_device::write_cs0));
	map(0x0300, 0x03af).noprw();
	map(0x03b0, 0x03df).noprw();
//  map(0x0278, 0x027b).w(FUNC(quakeat_state::pnp_config_w));
//  map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_device::read_cs1), FUNC(ide_controller_device::write_cs1));
//  map(0x0a78, 0x0a7b).w(FUNC(quakeat_state::pnp_data_w));
//  map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_device::read), FUNC(pci_bus_device::write));
}

/*************************************************************/

static INPUT_PORTS_START( quake )
INPUT_PORTS_END

/*************************************************************/

void quakeat_state::machine_start()
{
}
/*************************************************************/

void quakeat_state::quake(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM2(config, m_maincpu, 233000000); /* Pentium II, 233MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &quakeat_state::quake_map);
	m_maincpu->set_addrmap(AS_IO, &quakeat_state::quake_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(quakeat_state::screen_update_quake));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100);
}


ROM_START(quake)
	ROM_REGION32_LE(0x20000, "pc_bios", 0)  /* motherboard bios */
	ROM_LOAD("quakearcadetournament.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "quakeat", 0, SHA1(c44695b9d521273c9d3c0e18c88f0dca0185bd7b) )
ROM_END

} // anonymous namespace


GAME( 1998, quake,  0,   quake, quake, quakeat_state, empty_init, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", MACHINE_IS_SKELETON )
