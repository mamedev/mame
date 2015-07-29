// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/* Come On Baby
  (c) 2000 ExPotato Co. Ltd (Excellent Potato)

TODO:
Nearly everything.

  There also appears to be a sequel which may be running on the same hardware, but which does not seem to have been released.
  Come On Baby - Ballympic Heroes!  (c) 2001

  Other games in this series include:
  Come On Baby 2 (c) 2002
  Come On Baby Jr (c) 2003 (which seems to be otherwise identical to Come On Baby but in a smaller cabinet)
  Come On Baby 2 Jr (c) 2003 (which seems to be otherwise identical to Come On Baby 2 but in a smaller cabinet)
  These may or may not be on identical hardware.

  This is a Korean PC based board running Windows.  The game runs fully from
  the hard disk making these things rather fragile and prone to damage.

  PC Motherboard: PC Partner BXAS1-928
  BIOS String: 07/03/2000-440BX-ITE8671-2A69KV3IC-00
  aka. BIOS-I-2M
  Award BIOS
  B1120IAG.BIN  For Super IO = ITE 8671
        Clock Gen IC : Winbond-39A
        (Nov/2000 built)

  CPU - Slot 1 Celeron 633MHz (66x9.5)
  Memory - 65536k SDRAM PC100

  Brief motherboard overview (from PC Partner BXAS1-928 motherboard manual)
  --

  HARDWARE CONFIGURATION
  This motherboard is based on Intel 82440BX chipset. The chipset is a
  highly integrated solution for a cost-effective and compact motherboard.
  The motherboard supports standard DIMM module, PC-100 DIMM module or
  Registered DIMM Module.

  Features on-board include....
  super-I/O, Ultra DMA33 (Ultra DMA66 optional for VIA chipset), PCI bus master IDE, AGP Ver 1.0, PCI Ver 2.1 compliance,
  USB, VRM 8.4 compliance, ECC, ATX specification 2.01 compliance, hardware monitoring (optional), On-board PCI Sound
  Sub-system(optional, not populated). Supports PC-100 DIMM Module.

  Key Features:

  Processor
  - Full support for the Intel Pentium II & Intel Pentium III, Intel Celeron and Deschutes processors using Slot 1 connector.
  - Slot 1 connector for Intel Pentium II & Intel Pentium III microprocessors.
  - Supports 66MHz and 100MHz bus speed including all Pentium II & Pentium III processors and future processor.

  CPU Speed Setting
  - Jumper setting or no jumper is needed to set for various speed of CPU (Factory optional).

  VRM (Voltage Regulator Modules) on Board
  - Flexible motherboard design with on-board VRM 8.4, easy to upgrade with
  Future Intel Pentium II & Pentium III processors.

  Cache
  - Pentium II & Pentium III Processor built-in L2 cache.

  System Memory
  - Up to 384MB (SDRAM) or 768MB (Registered SDRAM) - for 440BX.
  - A total of three 168-pin DIMM sockets for 440BX.
  - Supports SDRAM (only for 66MHz bus speed).
  - Supports PC-100.

  Memory Organization
  - Supports single-density DIMMs of 1MB, 2MB, 4MB, 8MB and 16MB depth (x64 or 72).
  - Supports double-density DIMMs of 2MB, 4MB, 8MB, 16MB and 32MB depth
  (x64 or 72).
  - Supports error checking correction (ECC) using parity DRAM modules (for 440BX).
  - Banks of different DRAM types depths can be mixed.

  Expansion Slots
  - Supports SB-LINK Port for Creative Sound PCI Card.
  - 1 AGP slot (ver. 1.0, 1x/2x mode supported).
  - 5 PCI bus master slots (rev. 2.1 compliant, with 1 PCI slot sharing with 1 ISA slot)
  - 2 ISA slots (1 ISA slot sharing with 1 PCI slot).

  On-Board I/O
  - Two PCI fast IDE ports supporting up to 4 ATA2, Ultra DMA33 IDE HDDs, Ultra DMA66 (optional only for VIA Chipset)
  IDE HDDs, CD-Roms, ZIP devices and LS-120 drives as boot drive.
  - Supports bus master IDE, PIO mode 4 (up to 16M bytes/sec), Ultra DMA33 (up
  to 33M bytes/sec) transfer.
  - One ECP/EPP parallel port .
  - Two 16550-compatible UART serial ports.
  - One floppy port supporting two FDDs of 360KB, 720KB, 1.2MB, 1.44MB
  or 2.88MB formated capacity.
  - Two USB ports.
  - PS/2 keyboard port.
  - PS/2 mouse port.
  - Infrared (IrDA) support (via a header).
  - One Line / Speaker out, one Mic in, one Line in and MIDI / Gameport

  System BIOS
  - 2MB flash BIOS supporting PnP, APM, ATAPI, ACPI and DMI;
  - Jumper selection for 5V or 12V flash memory voltage.
  - Auto detects and supports LBA hard disks with formatted capacities over
  8.4GB.
  - Easily upgradable by end-user.

  Plug-and-Play
  - Supports Plug-and-Play Specification 1.1.
  - Plug-and-play for DOS, Windows 3.X, Windows 95 as well as Windows 98.
  - Fully steerable PCI interrupts.

  Power Management
  - Supports SMM, APM and ACPI.
  - Break switch for instant suspend/resume on system operation.
  - Energy star "Green PC" compliant .
  - Supports WAKE-ON-LAN (WOL).
  - Supports Wake on Ring for External Modem.
  - Supports ATX specification 2.01.

  Creative PCI Sound (optional, not populated)
  - Full DOS game support (DDMA, PC/PCI, CLS).
  - PCI 2.1 Bus Master, hardware sound acceleration.
  - Direct sound and sound Blaster Compatible.
  - Full Duplex, 3D Enhancement, Software wavetable.
  - PNP and APM 1.2 support.
  - Win95/98, NT drivers ready.

  Keyboard Password Turn ON
  - Special feature for system security.

  System monitoring (optional)
  - Hardware monitoring circuitry is supported, provides voltages, temperature, fan speeds etc. monitoring.

  --

  The donor PC looks like a standard Windows 98 setup.
  The only exceptions we see are that there's a game logo.sys/logo.bmp in the
  root directory to hide the Windows 98 startup screen, and a shortcut to
  the game in the startup programs.
  Also of interest, Windows 98 was installed from a setup folder on the HD.
  To me this hints that there may have been some expectation of the disk
  being plugged into random hardware.

  The game is pretty much a standard PC game running on a Windows 98 PC.
  It uses DirectSound and the Microsoft MCI interfaces and 3dfx Glide for video.
  The PC that the game was dumped from has Sound Blaster and Ensoniq drivers,
  but it works fine with some other sound configurations.
  The sound chip on the motherboard is not populated. There is a cheap Korean
  sound card "CS-6500P Made In Korea OJU CTN CO LTD." plugged into one of the
  slots containing a CRYSTAL CS4281-CM chip.
  The donor PC has a "3dfxvoodoo3" driver installation directory, but it works
  fine with a Voodoo4 4500.

  The game itself has some protection, it expects a file C:\\WINDOWS\win386p.swp of 84 bytes
  to have the hard disk volume serial number in ascii number (not hex) format at offset 4.

  The game appears to use parallel port (0x378,0x379) for I/O.
  The direct port access means it won't run on XP.
  For the controls, it writes a device select to 0x378, and reads the device value from 0x379.
  There is some other output, maybe lights?
   --------------------------------------------
  |SELECT|RETURN                               |
  |--------------------------------------------|
  | 0x8  | self test/protection, return 5      |
  | 0x0  | P1 4 way joystick                   |
  |      | ----x--- right                      |
  |      | -----x-- left                       |
  |      | ------x- down                       |
  |      | -------x up                         |
  | 0x1  | P1 buttons                          |
  |      | -----x-- C+D (is also start button) |
  |      | ------x- B                          |
  |      | -------x A                          |
  | 0x2  | P2 joystick (as P1)                 |
  | 0x3  | P2 buttons  (as P1)                 |
  | 0x4  | Coin/Service                        |
  |      | -----x-- Coin                       |
  |      | ------x- Coin                       |
  |      | -------x Test                       |
   --------------------------------------------

  Easy enough to fix a broken game if you have the controls to plug into it.
*/


#include "emu.h"
#include "cpu/i386/i386.h"


class comebaby_state : public driver_device
{
public:
	comebaby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
};


void comebaby_state::video_start()
{
}

UINT32 comebaby_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( comebaby_map, AS_PROGRAM, 32, comebaby_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( comebaby )
INPUT_PORTS_END


static MACHINE_CONFIG_START( comebaby, comebaby_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 2000000000) /* Probably a Pentium .. ?? Mhz*/
	MCFG_CPU_PROGRAM_MAP(comebaby_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(comebaby_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
MACHINE_CONFIG_END


ROM_START(comebaby)
	ROM_REGION32_LE(0x80000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("b1120iag.bin", 0x000000, 0x40000, CRC(9b6f95f1) SHA1(65d6a2fea9911593f093b2e2a43d1534b54d60b3) )

	DISK_REGION( "disks" )
	DISK_IMAGE( "comebaby", 0, SHA1(ea57919319c0b6a1d4abd7822cff028855bf082f) )
ROM_END


GAME( 2000, comebaby,  0,   comebaby,  comebaby, driver_device,  0,  ROT0,  "ExPotato",    "Come On Baby",   MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
