// license:BSD-3-Clause
// copyright-holders:Scott Stone

/* Global VR - PC-Based Hardware

Skeleton only at this time holding info regarding Install Disks
for Games/Operating System for Global VR produced games.
Specific hardware outlays are different per game, and a particular game can have
different motherboard setups: https://service.globalvr.com/reference/default.htm

The install disks contains PowerQuest DeployCenter 5.0 images.
Those aren't El Torito compliant so they expect at very least a
bootable MSDOS 5.0 (according to Pq_debug.txt file found in gvrxpsys).
The nfsgt is a Windows XP HDD image, containing:
- C:\Program Files:
\- C-Media 3D Audio driver (C-Media AC97 Audio Device / CMI8738/C3DX PCI Audio Device)
\- Intel 82865G/PE/P, 82875P (GMCH)
\- Ligos Indeo XP codec package (Indeo Video 5.2)
\- ALi USB2.0 Driver
- C:\temp
\- nVidia Display Driver 45.23 for Windows 2000 to XP (GeForce 256 up to GeForce 4)
\- DirectX 9.0
- C:\windows:
\- aksusb.inf: Aladdin Knowledge Systems HASP/Hardlock USB driver
\- akspccard.inf: Aladdin Knowledge Systems Hasp & Hardlock PCMCIA (PC-Card)
- C:\windows\inf:
\- 865.inf: driver for 8265
\- ich5core.inf/ich5ide.inf: Intel 82801EB Ultra ATA Storage Controllers
- C:\Gvr:
\- A full install of Need For Speed: Hot Pursuit 2
- C:\GvrRoot:
\- Data for nfsgt (overlay for above?), including screen for NoDongle.
- Footprints in C:\Documents and Settings\Administrator, reported just for
  completeness sake (read: likely not important):
\- "Local Settings\temp" for an unknown installer data
\- "Temporary Internet Files" for a failed Microsoft download log.


TODO:
- nfsgt hard disk crashes MAME with uncaught exception in both pcipc and shutms11


GlobalVR games on this hardware setups:

Game                                             | Year | I/O PCB               | Compatible MBs           | Compatible recovery disks
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Aliens: Extermination                            | 2006 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       |                          | 050-0227-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0220-01 or later
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0190-01 or 050-0198-01* or 050-0208-01*
                                                 |      |                       | Intel DG41TX             |
                                                 |      |                       |                          |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DQ45CB             | 050-0190-0
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | GA-945GM-S2              | 050-0166-01 or 050-0208-01*
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | ASUS P5KPL-CM            | 050-0166-01
                                                 |      |                       | 915GAG                   |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
America's Army                                   | 2007 | usb2gun               | GA-945GCMX-S2            | 050-0138-01
                                                 |      |                       | GA-945GM-S2              |
                                                 |      |                       | GA-G31MX-S2              |
                                                 |      |                       | Intel 915GAG             |
                                                 |      |                       | MSI K9NGM3               |
                                                 |      |                       | MSI K9NGM4               |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Beach Head 2000                                  | 2000 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Beach Head 2002                                  | 2002 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Beach Head 2003 Desert War                       | 2003 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Big Red Button                                   | 2006 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Blazing Angels                                   | 2008 | GVRI/O Mini           | MSI H61M-P32             | 050-0239-01
                                                 |      |                       |                          | 050-0229-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0222-01 or 050-0229-01
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0189-01 or 050-0202-01*
                                                 |      |                       | Intel DG41TX             |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Desert Gunner                                    | 2006 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports Madden NFL Football                    | 2004 | Nytric USBI/O Extreme | PS35-BL                  | 050-0094-01 (Season 2)
                                                 |      | GVRI/O Mini           |                          | 050-0057-01 (Version 1)
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports Madden NFL Season 2                    | 2006 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports NASCAR Racing                          | 2007 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports PGA Tour Golf                          | 2003 | Nytric USBI/O         | NB-32                    | 50-0024-01 REV B (Version 1)
                                                 |      |                       | PS35-BL                  |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports PGA Tour Golf Challenge Edition        | 2005 | Nytric USBI/O         | NB-32                    | 050-0072-01
                                                 |      |                       | PS35-BL                  |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports PGA Tour Golf Championship II Edition  | 200? | Nytric USBI/O         | NB-32                    | unknown
                                                 |      |                       | PS35-BL                  |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports PGA Tour Golf Championship III Edition | 200? | Nytric USBI/O         | NB-32                    | 050-0055-01
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports PGA Tour Golf Team Challenge           | 2006 | USBI/O Extreme or     | NB-32                    | 050-0097-01
                                                 |      | GVRI/O Mini           | PS35-BL                  |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
EA Sports Tiger Woods PGA Tour 2002              | 2002 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Fast Draw Showdown                               | 2002 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Feeding Frenzy                                   | 2006 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Frightfearland                                   | 2011 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       |                          | 050-0227-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0220-01 or later
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0198-01 or 050-0208-01*
                                                 |      |                       | Intel DG41TX             |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Global Arcade Classics                           | 2007 | USBlinx II            | Asus P5PE-VM             | 050-0158-01
                                                 |      |                       | GA-945GCMX-S2            |
                                                 |      |                       | Intel 915GAG             |
                                                 |      |                       | Gigabyte 81865GME-775-RH |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Gone Bowling                                     | 2006 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Gone Fishing                                     | 2006 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Heavy Gear                                       | 1999 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Heavy Gear II                                    | 1999 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Incoming                                         | 200? | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Invasion Earth                                   | 2003 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Justice League Heroes United                     | 2009 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | ASUS P5KPL-CM            | 050-0174-01
                                                 |      |                       | P5N-MX                   |
                                                 |      |                       | GA-G31MX-S2              |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Kick-It Pro                                      | 2006 | unknown               | D810E2CB                 | Not Applicable
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Last Bounty Hunter                               | 2002 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Mad Dog McCree                                   | 2002 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Mad Dog McCree 2                                 | 2002 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
NASCAR Racing                                    | 2010 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       |                          | 050-0227-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0220-01 or later
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0198-01* or 050-0208-01* plus Digital Vibrance Patch Disk 050-0207-01
                                                 |      |                       | Intel DG41TX             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | GA-G31MX-S2              | 050-0157-01 (Ver. 1.5)
                                                 |      |                       | GA-945GM-S2              | 050-0137-01 (Ver. 1)
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
NASCAR Team Racing                               | 2010 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       |                          | 050-0227-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0220-01 or later
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0185-01
                                                 |      |                       | Intel DG41TX             |
                                                 |      |                       | Intel DQ45CB             |
                                                 |      |                       | Intel DG31PR             |
                                                 |      |                       | GA-G31MX-S2              |
                                                 |      |                       | GA-945GM-S2              |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Need For Speed                                   | 2003 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Need For Carbon                                  | 2008 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      | Nytric USBI/O         +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0210-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0225-01 or 050-0184-01 Plus Digital Vibrance Patch: 050-0183-01
                                                 |      |                       | Intel DG41TX             |
                                                 |      |                       | Intel DQ45CB             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | P5N-MX                   | 050-0152-01
                                                 |      |                       | GA-G31MX-S2              |
                                                 |      |                       | Intel DG31PR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | PS35-BL                  | 050-0149-01
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Need For Speed GT                                | 2004 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Need For Speed Underground                       | 2005 | Nytric USBI/O         | MSI H61M-P32 (750 Video) | 050-0240-01
                                                 |      | GVRI/O Mini           +--------------------------+----------------------------
                                                 |      |                       | PS35-BL                  | 050-0070-01
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Operation Blockade                               | 200? | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Paradise Lost                                    | 2007 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0223-01
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0192-01
                                                 |      |                       | Intel DG41TX             |
                                                 |      |                       | Intel DQ45CB             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | ASUS P5KPL-CM (Intel G31)| 050-0126-01
                                                 |      |                       | GA-G31MX-S2              |
                                                 |      |                       | 915GAG                   |
                                                 |      |                       | Gigabyte GA-945GME-DS2   |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Puck Off                                         | 2007 | GFX I/O               | GA-945GCMX-S2            | 050-0131-01
                                                 |      |                       | GA-G31MX-S2              |
                                                 |      |                       | Gigabyte GA-945GM-S2     |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Redline Rampage Gas Guzzlers                     | 2014 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Six-Gun Select                                   | ???  | N/A                   | CS35TL                   | 050-0009-01 REV D
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Sin                                              | 1999 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Sky Bandits                                      | 2008 | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Split The Uprights                               | 200? | unknown               | unknown                  | unknown
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
The Swarm                                        | 2013 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       |                          | 050-0227-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | DFI EL339-b              | 050-0220-01 or later
                                                 |      |                       | Intel DH61CR             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TX             | 050-0218-01
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Twisted - Nitro Stunt Racing                     | 2009 | GVRI/O Mini           | MSI H61M-P32             | 050-0233-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DH61CR             | 050-0221-01
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Intel DG41TY             | 050-0179-01 or 050-0203-01* plus Digital Vibrance Patch Disk 050-0206-01
                                                 |      |                       | Intel DG41TX             |
                                                 |      |                       | Intel DQ45CB             |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
UltraPin                                         | 2006 | uShock I/O            | GA-945GM-S2              | 050-0114-01
                                                 |      |                       | Gigabyte GA-945GME-DS2   |
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
Vortek V3 & gvrSX                                | ???  | HAPP UGCI             | Intel DG41TY             | 050-0171-01 & patch disc 050-0191-01
                                                 |      |                       | Intel DQ45CB             |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | CS35TL                   | 050-0162-01
                                                 |      |                       | DFI GIC68-D              |
                                                 |      |                       +--------------------------+----------------------------
                                                 |      |                       | Graphite                 | 050-0019-01 REV A
-------------------------------------------------+------+-----------------------+--------------------------+----------------------------
        * Later recovery discs contain drivers for newer video cards.
*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class globalvr_state : public driver_device
{
public:
	globalvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void globalvr(machine_config &config);
private:
	required_device<cpu_device> m_maincpu;

	void globalvr_map(address_map &map) ATTR_COLD;
};



void globalvr_state::globalvr_map(address_map &map)
{
}


static INPUT_PORTS_START( globalvr )
INPUT_PORTS_END


void globalvr_state::globalvr(machine_config &config)
{
	// TODO: identify CPU socket
	// Logs inside gvrxpsys claims that it expects a "GenuineIntel"
	// with CPU features 0x0383f9ff (no SSE2, MMX, SSE, no Procesor Serial Number)
	// Socket 370 Celeron/Pentium 3?
	PENTIUM3(config, m_maincpu, 100'000'000);      // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &globalvr_state::globalvr_map);
	m_maincpu->set_disable();

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START( hyperv2 )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "hyperv2_pqi_6-12-02", 0, SHA1(44473f2950c0e108acb0961579a46f4765e379f7) )
ROM_END

ROM_START( hyperv2a )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x168000, "bootdisk", 0 ) // Win98/DOS bootdisk from folder made into .IMA with WinImage
	// "not-bootable system disk", but contains autoexec.bat / config.sys ...
	ROM_LOAD( "hyperv2_pqi_9-30-01.ima", 0x000000, 0x168000, BAD_DUMP CRC(964d8e00) SHA1(efefcfcca85328df8445a4ba482cd7d5b584ae05) )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "hyperv2_pqi_9-30-01", 0, SHA1(7a8c201a83a45609d0242a20441891f5204d7dd1) )
ROM_END

ROM_START( gvrxpsys )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "globalvr_xp_system", 0, SHA1(83a784fe038acbd651544b3fa3b17ceb11bbeeab) )
ROM_END

ROM_START( gvrxpsup )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "vr_xp_system_6-11-2002", 0, SHA1(c2b586a0106632bcaddc1df8077ee9c226537d2b) )
ROM_END

ROM_START( bhead2k )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "beachhead2000_5-27-2003", 0, SHA1(d4473a7fb9820f2e517a1e0609ec9e12f326fc06) )
ROM_END

ROM_START( bhead2ka )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "beachhead2000_9-16-2001", 0, SHA1(2151c0aff39a5279adb422e97f00c610d21c48e8) )
ROM_END

ROM_START( bhead2k2 )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "beachhead2002_5-27-2003", 0, SHA1(c58e62363387b76b4f03432b543498d4560d27a9) )
ROM_END

ROM_START( bhead2k3 )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:1:cdrom" )
	DISK_IMAGE_READONLY( "beachhead2003desertwar_5-27-2003", 0, SHA1(fed23a6496836050eb1d4f69b91da09adbd9d973) )
ROM_END

ROM_START( nfs )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	// HDD dumps, likely belonging to individual HDD with separate partitions
	DISK_REGION( "drive_1" )
	DISK_IMAGE( "need for speed disk 1 version 1.0.1 rev b", 0, SHA1(799017103c46712534e4bd9c04695fb8241a7ba4) )

	DISK_REGION( "drive_2" )
	DISK_IMAGE( "need for speed disk 2 version 1.0.1 rev b", 0, SHA1(800d1786bb9d2a2448c03c19ea6626af487aed90) )

	DISK_REGION( "recovery" )
	DISK_IMAGE( "emergency recovery disk 11.11.2003 rev a", 0, SHA1(38656b9da94150e5e8ed8a4183d2cc149e96aedd) )
ROM_END

ROM_START( nfsgt )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "need for speed gt", 0, SHA1(58bb2b47e30b65f2f09d2c2f2d7f300cf420b18a) )

	// CD-ROMs
	DISK_REGION( "drive_1" )
	DISK_IMAGE_READONLY( "need for speed gt disk 1 1.1.0 rev c", 0, SHA1(49d967a808f415d3ceb59a5758ee5b3fc4cfb551) )

	DISK_REGION( "drive_2" )
	DISK_IMAGE_READONLY( "need for speed gt disk 2 1.1.0 rev c", 0, SHA1(abbae9e61936079112c25c2b7bf2bbb608345ed2) )
ROM_END

ROM_START( nfsug )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("mb.bios", 0x00000, 0x80000, NO_DUMP )

	// CD-ROMs
	DISK_REGION( "drive_1" )
	DISK_IMAGE_READONLY( "nfsug1_1-disc1", 0, SHA1(25a9f0606ac3909bd7c4f3f3a59c6782e3c84712) )

	DISK_REGION( "drive_2" )
	DISK_IMAGE_READONLY( "nfsug1_1-disc2", 0, SHA1(5b0be45eb3fcd27ba513baca1da633f9e9a4c5ef) )

	DISK_REGION( "recovery" )
	DISK_IMAGE_READONLY( "nfsug-recovery", 0, SHA1(e306bacb3498582a025706ff81a665776b8a18da) )
ROM_END

} // anonymous namespace


// OS/Global VR specific Setup Installers
GAME( 2002, hyperv2,  0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Hyper V2 (Global VR) Install - 06/12/02",   MACHINE_IS_SKELETON )
GAME( 2001, hyperv2a, 0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Hyper V2 (Global VR) Install - 09/30/01",   MACHINE_IS_SKELETON )
GAME( 2001, gvrxpsys, 0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Global VR XP OS Install - 09/30/01",        MACHINE_IS_SKELETON )
GAME( 2002, gvrxpsup, 0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Global VR XP OS Update/Install - 06/11/02", MACHINE_IS_SKELETON )

// Game Installer CDs
GAME( 2000, bhead2k,  0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Beach Head 2000 Install - 05/27/03",                   MACHINE_IS_SKELETON )
GAME( 2000, bhead2ka, 0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Beach Head 2000 Install - 09/16/01",                   MACHINE_IS_SKELETON )
GAME( 2002, bhead2k2, 0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Beach Head 2002 Install - 05/27/03",                   MACHINE_IS_SKELETON )
GAME( 2003, bhead2k3, 0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Beach Head 2003 Desert War Install - 05/27/03",        MACHINE_IS_SKELETON )
GAME( 2003, nfs,      0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Need for Speed - 4 Cab Link (2 Discs) (v1.0.1 Rev B)", MACHINE_IS_SKELETON )
GAME( 2004, nfsgt,    0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Need for Speed GT (Hard Drive+2 Discs) (v1.1.0 Rev C)",MACHINE_IS_SKELETON )
GAME( 2005, nfsug,    0, globalvr, globalvr, globalvr_state, empty_init, ROT0, "Global VR", "Need For Speed: Underground Install (2 Discs) (v1.1)", MACHINE_IS_SKELETON )
