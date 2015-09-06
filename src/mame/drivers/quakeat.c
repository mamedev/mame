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


class quakeat_state : public pcat_base_state
{
public:
	quakeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		{ }

	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_quake(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


void quakeat_state::video_start()
{
}

UINT32 quakeat_state::screen_update_quake(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( quake_map, AS_PROGRAM, 32, quakeat_state )
	AM_RANGE(0x00000000, 0x0000ffff) AM_ROM AM_REGION("pc_bios", 0) /* BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( quake_io, AS_IO, 32, quakeat_state )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
//  AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs0, write_cs0, 0xffffffff)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
//  AM_RANGE(0x0278, 0x027b) AM_WRITE(pnp_config_w)
//  AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs1, write_cs1, 0xffffffff)
//  AM_RANGE(0x0a78, 0x0a7b) AM_WRITE(pnp_data_w)
//  AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_device, read, write)
ADDRESS_MAP_END

/*************************************************************/

static INPUT_PORTS_START( quake )
INPUT_PORTS_END

/*************************************************************/

void quakeat_state::machine_start()
{
}
/*************************************************************/

static MACHINE_CONFIG_START( quake, quakeat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM2, 233000000) /* Pentium II, 233MHz */
	MCFG_CPU_PROGRAM_MAP(quake_map)
	MCFG_CPU_IO_MAP(quake_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD( pcat_common )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(quakeat_state, screen_update_quake)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)

MACHINE_CONFIG_END


ROM_START(quake)
	ROM_REGION32_LE(0x20000, "pc_bios", 0)  /* motherboard bios */
	ROM_LOAD("quakearcadetournament.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "quakeat", 0, SHA1(c44695b9d521273c9d3c0e18c88f0dca0185bd7b) )
ROM_END


GAME( 1998, quake,  0,   quake, quake, driver_device, 0, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", MACHINE_IS_SKELETON )
