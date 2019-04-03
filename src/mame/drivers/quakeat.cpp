// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*

 Quake Arcade Tournament

 This is unknown PC hardware, only the HDD is dumped.  The HDD is stickered 'Release Beta 2'

 I've also seen CDs of this for sale, so maybe there should be a CD too, for the music?

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


 -- Hardware info found in the following press release:
http://www.wave-report.com/archives/1998/98170702.htm

QUANTUM3D'S HEAVY METAL SYSTEM - HM233G
NLX form factor system that is based on the Intel 440LX chipset
233MHz Intel Pentium II processor with 512KB of L2 cache
32MB of SDRAM
Microsoft Windows 95 OSR2.5
shock-mounted 3.1GB Ultra DMA-33 hard drive
12-24x CD-ROM drive
1.44 MB floppy drive
16-bit per sample 3D audio
PCI-based 2D/VGA
built-in 10/100 Ethernet
Obsidian2 90-4440 AGP Voodoo2-based realtime 3D graphics accelerator
Quantum3D's GCI (Game Control Interface) - a unique, low-cost subsystem
    designed to interface coin-op and industrial input/output control devices to a PC

===============================================================================
TODO:
    * Add BIOS dump (custom 440LX motherboard or standard?)
    * Hook up PC hardware
    * Hook up the GCI (details? ROMs?)
    * What's the dongle do?
===============================================================================
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pcshare.h"
#include "emupal.h"
#include "screen.h"


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
//  AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs0, write_cs0, 0xffffffff)
	map(0x0300, 0x03af).noprw();
	map(0x03b0, 0x03df).noprw();
//  AM_RANGE(0x0278, 0x027b) AM_WRITE(pnp_config_w)
//  AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs1, write_cs1, 0xffffffff)
//  AM_RANGE(0x0a78, 0x0a7b) AM_WRITE(pnp_data_w)
//  AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_device, read, write)
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


GAME( 1998, quake,  0,   quake, quake, quakeat_state, empty_init, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", MACHINE_IS_SKELETON )
