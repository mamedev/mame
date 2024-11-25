// license:BSD-3-Clause
// copyright-holders:David Haywood
// thanks-to: sampson
/* Taito Type X Skeleton
 - PC based platforms

 (insert original hardware specs, plus any other details here)
 https://wiki.arcadeotaku.com/w/Taito_Type_X
 https://wiki.arcadeotaku.com/w/Taito_Type_X%C2%B2

TODO:
 - Undumped custom BIOSes, at least regular Type X uses a Springdale spinoff.
   Type X: Intel 865G
   Type X+: as above plus better PCI video/sound cards
   Type X7: Intel 855GME + ICH4
   Type X2 & Satellite Terminal: Intel Q965 + ICH8
   Type X Zero: MCP7A-ION
   Type X3: Intel Q67 express

 - GPUs also uses custom BIOSes, again undumped;

 - To access BIOS menu needs to hold CTRL+ALT+F9 on POST. The menu is password
   protected in plaintext ...

 - The BIOS cannot load any OS that isn't the intended ones, non canonical MBR
   checks? investigate;

 - there are a lot of hacked versions of the games designed to run on PCs
   this driver is for storing information about the unmodified versions only,
   the 'hacked to run on PC' versions are of no interest to MAME, only proper
   drive images.

 - For copy protection most if not all games uses a USB dongle with sim card that
   is necessary for decrypting game containers inside HDD cfr. page 12 of the Type X2
   manual

 - (old note) some of the games are said to be encrypted, how does this work,
   how do we get the keys? some images are copies of the files from an already mounted
   filesystem, again this isn't suitable for MAME.

 - Taito's NESiCAxLive platform requires a live connection to a dedicated
   Taito Type X Zero in-store "server", that will pass the info to a intranet
   connected Type X/X2. Notice that the Type X Zero will eventually try to connect thru
   "strict" ssl pinning, which will refuse any connection that isn't the internally
   defined CA.

 - Type X Zero can also be used on specific cab setups, for example connecting two
   Type X3 usf4 for versus mode.

 - Preliminary game lists (mainly from system16.com)

    Taito Type X games

    Chaos Breaker / Dark Awake
    Data Carddass Dragon Ball Z
    Dinoking III Allosaurus
    Dinomax
    Dragon Quest - Monster Battle Road
    Dragon Quest - Monster Battle Road II Legends
    Gigawing Generations
    Gouketsuji Ichizoku: Senzo Kuyou
    Harakari Professional Baseball
    Homura
    King Of Jurassic
    KOF Sky Stage
    Matrimelee Matsuri / Power Instinct V
    Raiden III
    Raiden IV
    Shikigami No Shiro III / The Castle of Shikigami III
    Spica Adventure
    Taisen Hot Gimmick 5
    Taisen Hot Gimmick Mix Party
    Tetris The Grand Master 3 - Terror-Instinct
    The King of Fighters 98 Ultimate Match
    Trouble Witches AC
    Usagi Online
    Valve Limit R
    Zoids Card Colosseum

    Taito Type X+ games

    Battle Gear 4
    Battle Gear 4 Tuned
    Half Life 2 Survivor
    Mobile Suit Gundam - Spirits of Zeon Senshi no Kioku
    War Of The Grail

    Taito Type X2 games

    Battle Fantasia
    BlazBlue: Calamity Trigger
    BlazBlue: Chrono Phantasma
    BlazBlue: Continuum Shift
    BlazBlue: Continuum Shift Extend
    BlazBlue: Continuum Shift II
    Chase H.Q. 2 / Chase H. Q. : Nancy Yori Kinkyuu Renraku
    Chou Chabudai Gaeshi!
    Chou Chabudai Gaeshi! 2
    Chou Chabudai Gaeshi! Kyojin No Hoshi
    Cyber Diver
    D1GP Arcade
    Dariusburst - Another Chronicle
    Dariusburst - Another Chronicle EX
    Elevator Action Death Parade
    Gunslinger Stratos
    Half Life 2: Survivor Ver. 2.0
    Haunted Museum / Panic Museum
    Haunted Museum II / Panic Museum II
    Hopping Kids
    Hopping Road
    KOF Maximum Impact: Regulation A
    KOF Maximum Impact: Regulation A2
    Lord Of Vermilion Re:2
    Matrimelee Matsuri / Power Instinct V
    P4U - Persona 4 The Ultimate In Mayonaka Arena
    P4U - Persona 4 The Ultimax Ultra Suplex Hold
    Samurai Spirits Sen / Samurai Shodown: Edge of Destiny
    Senko no Ronde DUO : Dis-United Order
    Street Fighter IV
    Super Street Fighter IV Arcade Edition
    The King of Fighters XII
    The King of Fighters XIII
    The King Of Fighters XIII Climax
    Top Speed
    Wacky Races

    Taito Type X2 satellite terminal games

    Aquarian Age Alternative
    Eternal Wheel
    Lord of Vermilion
    Lord of Vermilion II

    Taito Type X Zero games

    Card de Renketsu! Densha de Go!
    Groove Coaster
    Groove Coaster 2 Heavenly Festival
    Groove Coaster 3 Link Fever
    Groove Coaster 3 EX Dream Party
    Groove Coaster 4 Starlight Road
    Groove Coaster 4 EX Infinity Highway
    Groove Coaster 4 MAX Diamond Galaxy
    Groove Coaster EX
    Kickthrough Racers
    Mogutte Horehore
    Spin Gear

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class taito_type_x_state : public driver_device
{
public:
	taito_type_x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void taito_type_x(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void taito_type_x_map(address_map &map) ATTR_COLD;
};


void taito_type_x_state::taito_type_x_map(address_map &map)
{
}

static INPUT_PORTS_START( taito_type_x )
INPUT_PORTS_END

// todo: different configs for the different machine types.
void taito_type_x_state::taito_type_x(machine_config &config)
{
	// Socket 478
	PENTIUM4(config, m_maincpu, 100'000'000); /* Wrong, much newer processors, much faster. */
	m_maincpu->set_addrmap(AS_PROGRAM, &taito_type_x_state::taito_type_x_map);
	m_maincpu->set_disable();

	PCI_ROOT(config, "pci", 0);
	// ...
}

} // anonymous namespace


/***************************************************************************

  Game drivers

***************************************************************************/

// Type X

ROM_START( chaosbrk )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "chaosbreaker_v2_02j", 0, SHA1(8fe7bdc20a8d9e81f08cef60324ed9ac978a16e9) )
ROM_END

ROM_START( goketsuj )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 160GB drive
	DISK_IMAGE( "goketsuji_v200906230", 0, SHA1(f0733fbb42994208e18c6afe67f1e9746351a3a2) )
ROM_END

ROM_START( gwinggen )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "gigawing_v2_02j", 0, SHA1(e09a2e5019111765689cb205cc94e7868c55e9ca) )
ROM_END

ROM_START( homura )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "homura_v2_04jpn", 0, SHA1(0d9d24583fa786b82bf27447408111bd4686033e) )
ROM_END

ROM_START( hotgmkmp )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "wdc wd400eb-11cpf0", 0, SHA1(15f8cf77b5bdc516a891022462a42521be1d7553) )
ROM_END

ROM_START( kof98um )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "kof98um_v1_00", 0, SHA1(cf21747ddcdf802d766a2bd6a3d75a965e89b2cf) )
ROM_END

ROM_START( kofskyst)
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" )
	// Single 40GB drive(KOF SKY STAGE - M9008134A - Ver.1.00J - 40.0 GB WD - WD400BB-22JHCO)
	// d1337c5ee363a6a95a9d1caf45857993 kof_sky_stage_ver.1.00j.img
	// 0x3b61ddf8 kof_sky_stage_ver.1.00j_ata_id.bin
	DISK_IMAGE( "kof_sky_stage_v1.00j", 0, SHA1(86bdd4c7cd6f198c07c125a65b5361afe1bdd199) )
ROM_END

ROM_START( raiden3 )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "raiden3_v2_01j", 0, SHA1(60142f765a0706e938b91cc41dc14eb67bd78615) )
ROM_END

ROM_START( raiden4 )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "raiden4_v1_00j", 0, SHA1(f5ad509f57067089e0217df6d05036484b06a41a) )
ROM_END

ROM_START( shikiga3 )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 20GB drive
	DISK_IMAGE( "shikigami3_v2_06jpn", 0, SHA1(4bf41ab1a3f2cd51cd2b1e6183959d2a4878449d) )
ROM_END

ROM_START( spicaadv )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	DISK_IMAGE( "spicaadventure_v2_03j", 0, SHA1(218ee0670a7b895f42480f0fe6719ecd4f4ba9e6) )
ROM_END

// stickers: CPU Cel 2.5 GHz  MEM 256 MB  GRA 9600XT
//           TAITO Type X  Model 006B   No 150FG0217  AC100V  50/60Hz 2.5A
//           Windows(R) XP Embedded    00039-111-243-487    X11-15305
//
//           USAGI   M9006613A  VER.2.04JPN     40.0GB  WD  WD400BB-22JHCO
ROM_START( usagiol )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive, WD Caviar model WD400BB, LBA 78165360
	DISK_IMAGE( "usagionline_v2_04j", 0, SHA1(6d4a780c40ee5c9b0192932e926f144d76b87262) )
ROM_END

ROM_START( trbwtchs )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 160GB drive
	DISK_IMAGE( "troublewitches_ac_v1.00j", 0, SHA1(733ecbae040dd32447230d3fc81e6f8614715ee5) )
ROM_END


GAME( 2004, chaosbrk,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Taito Corporation", "Chaos Breaker (v2.02J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2004, gwinggen,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Takumi Corporation", "Giga Wing Generations (v2.02J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2005, homura,    0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "SKonec Entertainment", "Homura (v2.04J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2005, hotgmkmp,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "XNauts", "Taisen Hot Gimmick Mix Party",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2005, raiden3,   0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "MOSS / Seibu Kaihatsu", "Raiden III (v2.01J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2005, spicaadv,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Taito Corporation", "Spica Adventure (v2.03J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2005, usagiol,   0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Taito Corporation/Warashi", "Usagi: Yasei no Topai Online (v2.04J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2006, shikiga3,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Alfa System/SKonec Entertainment", "Shikigami no Shiro III (v2.06J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2007, raiden4,   0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "MOSS / Seibu Kaihatsu", "Raiden IV (v1.00J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2008, kof98um,   0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "SNK", "The King of Fighters '98: Ultimate Match (v1.00)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2008, trbwtchs,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Adventure Planning Service/Studio SiestA", "Trouble Witches AC (v1.00J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2009, goketsuj,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Atlus", "Gouketsuji Ichizoku: Senzo Kuyou (v200906230)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2010, kofskyst,  0,    taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Moss / SNK Playmore", "KOF Sky Stage (v1.00J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )


// Type X+

// stickers: CPU P4 2.8 GHz  MEM 512 MB  GRA 9600XT
//           TAITO Type X+  Model 012A  No 470GM1941  AC100V  50/60Hz 3.5A
//           Windows(R) XP Embedded    00039-209-795-804    X11-15305
//
//           ****   M9006981A  VER.1.00   40.0GB  WD  WD400BB-22JHCO
ROM_START( wontmuch )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 40GB drive
	// 5BC6813ADBE1525BAFED792FC12C27AB
	DISK_IMAGE( "wontmuch_v1.00", 0, SHA1(fcc9719aa68234e3df5e8cb9cd7bcb1b0dcb66b1) )
ROM_END


GAME( 2006, wontmuch, 0, taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Capcom", "Won!Tertainment Music Channel (v1.00)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )


// Type X2

ROM_START( chasehq2 )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 80GB drive
	// 2188b8d76766c34580d99bf5ab0848fc 696092ff8467034acc4b34702006b3afbcb90082 chase_hq_2_v2.0.6.jp.001
	DISK_IMAGE( "chase_hq_2_v2.0.6.jp", 0, SHA1(900620e27c1465dff4ced4b6ae479356c9a785e0) )
ROM_END

ROM_START( samspsen )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 80GB drive
	//e19435da2cd417d1e2949d14a043d6e1 50ef9af80b11984c56e4f765a6f827fa5d22b404 samurai spirits sen.v1.00.001
	DISK_IMAGE( "samurai spirits sen.v1.00", 0, SHA1(5c687604066301a5b7c60f7fc778f0961efce0b6) )
ROM_END

ROM_START( kofxii )
	ROM_REGION32_LE( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	ROM_REGION( 0x1000, "dongle", ROMREGION_ERASEFF )
	ROM_LOAD("dongle.pic", 0, 0x1000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Single 80GB drive
	//2ff6b2d33ab7e915733a4328c73695de xii_ver.1.img
	DISK_IMAGE( "kof xii.v1.00", 0, SHA1(9caaca4cf4d3bacd218932004f0c761337ee8a6f) )
ROM_END


GAME( 2006, chasehq2, 0, taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "Taito Corporation", "Chase H.Q. 2 (v2.0.6.JP)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2008, samspsen, 0, taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "SNK Playmore",      "Samurai Spirits Sen (v1.00)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 2009, kofxii,   0, taito_type_x, taito_type_x, taito_type_x_state, empty_init, ROT0, "SNK Playmore",      "The King of Fighters XII (v1.00)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION )
