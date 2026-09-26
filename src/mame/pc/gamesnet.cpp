// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

GAMESnet Choice MK3 Combi-Client SWP

RadiSys Endura GL815E
- Intel FW82815 "Solano GMCH" northbridge
- Intel FW82801BA "ICH2" southbridge
- Intel DA82562ET "Kinnereth" ethernet
- National PC87366 Super I/O
- PC87200 for PCI-to-ISA bridge

"Gamesnet Aventa" also known to exist (probably an HW bump with similar if not same PCI
cards attached)

Heuristics, with ls5amvp3 (the only driver that can boot this in Safe Mode):
- Windows 2000 build 5.00.2195 Service Pack 2Free;
- Installs a CD-ROM and floppy drives on first boot, which implies they are unfitted on this config;
- C:\Millennium looks the main game folder (TBD)
- C:\Build contains various drivers and utilities
\- LogView.exe: "Remote Logging Viewer" program for a socket connection;
\- BellCard.exe: "GamesNet Card Tester" for the Bellfruit I/O card PCI;
\- \Microtouch: a package containing MicroTouch TouchWare 5.61;
\- \NTIODriver: glue logic for Windows?
\- \OX16PCI: Oxford Semiconductor multifunction (COM & LPT) installer for OX16PCI954/OX9162
   https://admin.pci-ids.ucw.cz/read/PC/1415
\- \Shockwave: a Flash Player 5/5AX;

TODO:
- stub for Intel 815E chipset;
- expose the RadiSys Endura BIOS as separate romset;

===================================================================================================

harddrive = Seagate ST320410A

Version 2.2

New hardware platform featuring:
- Intel gl815e main board on board graphics and sound
- Intel Pentium III 866 MHz CPU
- 256MB RAM
- Bell Fruit PCI I/O serial card
- Contemporary Controls PCI20X ARCnet card
- New Diamond SupraSST PCI v92 df  Modem
- On board Ethernet

OS installed is Windows 2000 (service pack 2)
DHTML1 B  Shell installed, non carrosell shell
10 games active including new content


Version 2.3

Drive defragmented with DiskKeeper and ScanDisk run.
Default keyboard driver installed to prevent dialogue on first boot.
---------------------------------------------------------------------------

that is probably wrong,
i believe it used a custom USB i/o card, (that i have)
and a microtouch serial touchscreen controller.

Highwayman.

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class gamesnet_state : public driver_device
{
public:
	gamesnet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void gamesnet(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void gamesnet_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x0009'ffff).ram();
	map(0x000e'0000, 0x000f'ffff).rom().region("bios", 0x60000);

	map(0xfff8'0000, 0xffff'ffff).rom().region("bios", 0);
}

void gamesnet_state::main_io(address_map &map)
{
}

void gamesnet_state::gamesnet(machine_config &config)
{
	// Socket 370 FSB 66 / 100 MHz
	PENTIUM3(config, m_maincpu, 100'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gamesnet_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &gamesnet_state::main_io);

	PCI_ROOT(config, "pci");
	// ...

	// 2x ISA16
	// 1x 3.3V AGP (unpopulated on this config)
	// 5x PCI

	// 512MB max RAM, SDRAM 168-pin DIMM
	// 32MB max VRAM

	// 1x 3.5mm Audio Combo
	// 1x Floppy interface
	// 1x Gameport
	// 2x IDE
	// 1x PS/2 keyboard and 1x Mouse
	// 1x Parallel port
	// 1x RJ-45 LAN
	// 2x Serial (probably unused here, routed thru the PCI card)
	// 2x USB 1.x
	// 1x VGA connector
}


// BIOS is not provided by the dumper, assume it's retail unmodified
ROM_START( gamesmil )
	ROM_REGION32_LE(0x80000, "bios", 0)
	// Phoenix BIOSes 4.0 Release 6.0
	// v2.03.11
	ROM_LOAD("ph815-2-03-11.bin", 0x000000, 0x80000, CRC(91f09c33) SHA1(9891732f348f63eedd0b38fff1cc0cdf6b9e3dac))
	// v2.03.10 also known to exist

	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("millennium", 0, SHA1(1bffe2cad85bccd68427133007dd70a4d9e3f1a9))
ROM_END

} // anonymous namespace


GAME(200?, gamesmil, 0, gamesnet, 0, gamesnet_state, empty_init, ROT0, "Games Network Limited", "Millennium (GAMESnet Choice MK3, Intel 815E Solano-2 chipset)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // Can't be earlier than 2001 out of using Windows 2000 SP2, title is unconfirmed
