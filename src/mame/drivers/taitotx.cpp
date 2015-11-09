// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Taito Type X Skeleton
 - PC based platforms

 (insert original hardware specs, plus any other details here)

 todo: everything..

 - there is no bios dump

 - there are a lot of hacked versions of the games designed to run on PCs
   this driver is for storing information about the unmodified versions only,
   the 'hacked to run on PC' versions are of no interest to MAME, only proper
   drive images.

 - some of the games are said to be encrypted, how does this work, how do we
   get the keys? some images are copies of the files from an already mounted
   filesystem, again this isn't suitable for MAME.

 - hardware specs can differ from game to game as well as between the platform
   types, I'm currently not sure what constitutes a new platform (different
   security?)  need Guru style readmes for each platform.

 - Taito's NESiCA Live platform probably comes after this, but as it's likely
   impossible to ever emulate it.

 - Preliminary game lists (mainly from system16.com)

    Taito Type X games

    Chaos Breaker / Dark Awake
    Datacarddass Dragon Ball Z
    Dinoking III Allosaurus
    Dinomax
    Dragon Quest - Monster Battle Road
    Dragon Quest - Monster Battle Road II Legends
    Gigawing Generations
    Goketsuji Ichizoku: Matsuri Senzo Kuyou
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

    Spin Gear

*/


#include "emu.h"
#include "cpu/i386/i386.h"

class taito_type_x_state : public driver_device
{
public:
	taito_type_x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_taito_type_x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};




void taito_type_x_state::video_start()
{
}


UINT32 taito_type_x_state::screen_update_taito_type_x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( taito_type_x_map, AS_PROGRAM, 32, taito_type_x_state )
	AM_RANGE(0x00, 0x0f) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( taito_type_x )
INPUT_PORTS_END


void taito_type_x_state::machine_start()
{
}

void taito_type_x_state::machine_reset()
{
}

// todo: different configs for the different machine types.
static MACHINE_CONFIG_START( taito_type_x, taito_type_x_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM3, 733333333) /* Wrong, much newer processors, much faster. */
	MCFG_CPU_PROGRAM_MAP(taito_type_x_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(taito_type_x_state, screen_update_taito_type_x)

	MCFG_PALETTE_ADD("palette", 0x10000)

MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

// Type X

ROM_START( chaosbrk )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "chaosbreaker_v2_02j", 0, SHA1(8fe7bdc20a8d9e81f08cef60324ed9ac978a16e9) )
ROM_END

ROM_START( goketsuj )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 160GB drive
	DISK_IMAGE( "goketsuji_v200906230", 0, SHA1(f0733fbb42994208e18c6afe67f1e9746351a3a2) )
ROM_END

ROM_START( gwinggen )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "gigawing_v2_02j", 0, SHA1(e09a2e5019111765689cb205cc94e7868c55e9ca) )
ROM_END

ROM_START( homura )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "homura_v2_04jpn", 0, SHA1(0d9d24583fa786b82bf27447408111bd4686033e) )
ROM_END

ROM_START( hotgmkmp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "wdc wd400eb-11cpf0", 0, SHA1(15f8cf77b5bdc516a891022462a42521be1d7553) )
ROM_END

ROM_START( kof98um )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "kof98um_v1_00", 0, SHA1(cf21747ddcdf802d766a2bd6a3d75a965e89b2cf) )
ROM_END

ROM_START( raiden3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "raiden3_v2_01j", 0, SHA1(60142f765a0706e938b91cc41dc14eb67bd78615) )
ROM_END

ROM_START( raiden4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "raiden4_v1_00j", 0, SHA1(f5ad509f57067089e0217df6d05036484b06a41a) )
ROM_END

ROM_START( shikiga3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 20GB drive
	DISK_IMAGE( "shikigami3_v2_06jpn", 0, SHA1(4bf41ab1a3f2cd51cd2b1e6183959d2a4878449d) )
ROM_END

ROM_START( spicaadv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	DISK_IMAGE( "spicaadventure_v2_03j", 0, SHA1(218ee0670a7b895f42480f0fe6719ecd4f4ba9e6) )
ROM_END

// stickers: CPU Cel 2.5 GHz  MEM 256 MB  GRA 9600XT
//           TAITO Type X  Model 006B   No 150FG0217  AC100V  50/60Hz 2.5A
//           Windows(R) XP Embedded    00039-111-243-487    X11-15305
//
//           USAGI   M9006613A  VER.2.04JPN     40.0GB  WD  WD400BB-22JHCO
ROM_START( usagiol )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive, WD Caviar model WD400BB, LBA 78165360
	DISK_IMAGE( "usagionline_v2_04j", 0, SHA1(6d4a780c40ee5c9b0192932e926f144d76b87262) )
ROM_END

ROM_START( trbwtchs )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 160GB drive
	DISK_IMAGE( "troublewitches_ac_v1.00j", 0, SHA1(733ecbae040dd32447230d3fc81e6f8614715ee5) )
ROM_END


GAME( 2004, chaosbrk,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Taito Corporation", "Chaos Breaker (v2.02J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2004, gwinggen,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Takumi Corporation", "Giga Wing Generations (v2.02J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, homura,    0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "SKonec Entertainment", "Homura (v2.04J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, hotgmkmp,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "XNauts", "Taisen Hot Gimmick Mix Party",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, raiden3,   0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "MOSS / Seibu Kaihatsu", "Raiden III (v2.01J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, spicaadv,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Taito Corporation", "Spica Adventure (v2.03J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, usagiol,   0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Taito Corporation/Warashi", "Usagi Online (v2.04J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2006, shikiga3,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Alfa System/SKonec Entertainment", "Shikigami no Shiro III (v2.06J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2007, raiden4,   0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "MOSS / Seibu Kaihatsu", "Raiden IV (v1.00J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2008, kof98um,   0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "SNK", "The King of Fighters '98: Ultimate Match (v1.00)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2008, trbwtchs,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Adventure Planning Service/Studio SiestA", "Trouble Witches AC (v1.00J)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2009, goketsuj,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Atlus", "Goketsuji Ichizoku: Matsuri Senzo Kuyou (v200906230)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )


// Type X+

// stickers: CPU P4 2.8 GHz  MEM 512 MB  GRA 9600XT
//           TAITO Type X+  Model 012A  No 470GM1941  AC100V  50/60Hz 3.5A
//           Windows(R) XP Embedded    00039-209-795-804    X11-15305
//
//           ****   M9006981A  VER.1.00   40.0GB  WD  WD400BB-22JHCO
ROM_START( wontmuch )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 40GB drive
	// 5BC6813ADBE1525BAFED792FC12C27AB
	DISK_IMAGE( "wontmuch_v1.00", 0, SHA1(fcc9719aa68234e3df5e8cb9cd7bcb1b0dcb66b1) )
ROM_END


GAME( 2006, wontmuch,   0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Capcom", "Won!Tertainment Music Channel (v1.00)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )


// Type X2

ROM_START( chasehq2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 80GB drive
	// 2188b8d76766c34580d99bf5ab0848fc 696092ff8467034acc4b34702006b3afbcb90082 chase_hq_2_v2.0.6.jp.001
	DISK_IMAGE( "chase_hq_2_v2.0.6.jp", 0, SHA1(900620e27c1465dff4ced4b6ae479356c9a785e0) )
ROM_END

ROM_START( samspsen )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x2_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "ide:0:hdd:image" ) // Single 80GB drive
	//e19435da2cd417d1e2949d14a043d6e1 50ef9af80b11984c56e4f765a6f827fa5d22b404 samurai spirits sen.v1.00.001
	DISK_IMAGE( "samurai spirits sen.v1.00", 0, SHA1(5c687604066301a5b7c60f7fc778f0961efce0b6) )
ROM_END


GAME( 2006, chasehq2,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "Taito Corporation", "Chase H.Q. 2 (v2.0.6.JP)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2008, samspsen,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "SNK Playmore", "Samurai Spirits Sen (v1.00)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
