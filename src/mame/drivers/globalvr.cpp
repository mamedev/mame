// license:BSD-3-Clause
// copyright-holders:Scott Stone
/* Global VR - PC-Based Hardware

Skeleton only at this time holding info regarding Install Disks
for Games/Operating System for Global VR produced games.
Specific hardware outlays are unknown per game at this time.

Game List                                        Year
-----------------------------------------------------
Aliens: Extermination                            2006
America's Army                                   2007
Beach Head 2000                                  2000
Beach Head 2002                                  2002
Beach Head 2003 Desert War                       2003
Big Red Button                                   2006
Blazing Angels                                   2008
Desert Gunner                                    2006
EA Sports Madden NFL Football                    2004
EA Sports Madden NFL Season 2                    2006
EA Sports NASCAR Racing                          2007
EA Sports PGA Tour Golf                          2003
EA Sports PGA Tour Golf Challenge Edition        2005
EA Sports PGA Tour Golf Challenge Edition II     200?
EA Sports PGA Tour Golf Challenge Edition III    200?
EA Sports PGA Tour Golf Team Challenge           2006
EA Sports Tiger Woods PGA Tour 2002              2002
Fast Draw Showdown                               2002
Feeding Frenzy                                   2006
FrightFearLand                                   2011
Global Arcade Classics                           2007
Gone Bowling                                     2006
Gone Fishing                                     2006
Heavy Gear                                       1999
Heavy Gear II                                    1999
Incoming                                         200?
Invasion Earth                                   2003
Justice League Heroes United                     2009
Kick-It Pro                                      2006
Last Bounty Hunter                               2002
Mad Dog McCree                                   2002
Mad Dog McCree 2                                 2002
NASCAR Team Racing                               2010
Need For Speed                                   2003
Need For Speed Carbon                            2008
Need For Speed GT                                2004
Need For Speed Underground                       2005
Operation Blockade                               200?
Paradise Lost                                    2007
Puck Off                                         2007
Sin                                              1999
Sky Bandits                                      2008
Split The Uprights                               200?
Twisted - Nitro Stunt Racing                     2009
UltraPin                                         2006
*/


#include "emu.h"
#include "cpu/i386/i386.h"

class globalvr_state : public driver_device
{
public:
	globalvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( globalvr_map, AS_PROGRAM, 32, globalvr_state )
	AM_RANGE(0x00000000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( globalvr )
INPUT_PORTS_END


static MACHINE_CONFIG_START( globalvr, globalvr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 100000000)      /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(globalvr_map)
MACHINE_CONFIG_END


ROM_START( hyperv2 )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "hyperv2_pqi_6-12-02", 0, SHA1(44473f2950c0e108acb0961579a46f4765e379f7) )
ROM_END

ROM_START( hyperv2a )
	ROM_REGION( 0x168000, "bootdisk", 0 ) /* Win98/DOS bootdisk from folder made into .IMA with WinImage */
	ROM_LOAD( "hyperv2_pqi_9-30-01.ima", 0x000000, 0x168000, CRC(964d8e00) SHA1(efefcfcca85328df8445a4ba482cd7d5b584ae05) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "hyperv2_pqi_9-30-01", 0, SHA1(7a8c201a83a45609d0242a20441891f5204d7dd1) )
ROM_END

ROM_START( gvrxpsys )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "globalvr_xp_system", 0, SHA1(83a784fe038acbd651544b3fa3b17ceb11bbeeab) )
ROM_END

ROM_START( gvrxpsup )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "vr_xp_system_6-11-2002", 0, SHA1(c2b586a0106632bcaddc1df8077ee9c226537d2b) )
ROM_END

ROM_START( bhead2k )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "beachhead2000_5-27-2003", 0, SHA1(d4473a7fb9820f2e517a1e0609ec9e12f326fc06) )
ROM_END

ROM_START( bhead2ka )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "beachhead2000_9-16-2001", 0, SHA1(2151c0aff39a5279adb422e97f00c610d21c48e8) )
ROM_END

ROM_START( bhead2k2 )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "beachhead2002_5-27-2003", 0, SHA1(c58e62363387b76b4f03432b543498d4560d27a9) )
ROM_END

ROM_START( bhead2k3 )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "beachhead2003desertwar_5-27-2003", 0, SHA1(fed23a6496836050eb1d4f69b91da09adbd9d973) )
ROM_END

ROM_START( nfsug )
	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "nfsug1_1-disc1", 0, SHA1(25a9f0606ac3909bd7c4f3f3a59c6782e3c84712) )

	DISK_REGION( "drive_1" )
	DISK_IMAGE_READONLY( "nfsug1_1-disc2", 0, SHA1(5b0be45eb3fcd27ba513baca1da633f9e9a4c5ef) )
ROM_END


/* OS/Global VR specific Setup Installers */
GAME( 2002, hyperv2,   0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Hyper V2 (Global VR) Install - 06/12/02", MACHINE_IS_SKELETON )
GAME( 2001, hyperv2a,  0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Hyper V2 (Global VR) Install - 09/30/01", MACHINE_IS_SKELETON )
GAME( 2001, gvrxpsys,  0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Global VR XP OS Install - 09/30/01", MACHINE_IS_SKELETON )
GAME( 2002, gvrxpsup,  0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Global VR XP OS Update/Install - 06/11/02", MACHINE_IS_SKELETON )

/* Game Installer CDs */
GAME( 2000, bhead2k,   0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Beach Head 2000 Install - 05/27/03", MACHINE_IS_SKELETON )
GAME( 2000, bhead2ka,  0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Beach Head 2000 Install - 09/16/01", MACHINE_IS_SKELETON )
GAME( 2002, bhead2k2,  0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Beach Head 2002 Install - 05/27/03", MACHINE_IS_SKELETON )
GAME( 2003, bhead2k3,  0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Beach Head 2003 Desert War Install - 05/27/03", MACHINE_IS_SKELETON )
GAME( 2005, nfsug,     0, globalvr, globalvr, driver_device, 0, ROT0, "Global VR", "Need For Speed: Underground Install (2 Discs) (v1.1)", MACHINE_IS_SKELETON )
