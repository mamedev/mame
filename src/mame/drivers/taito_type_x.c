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

 - Prelim game lists (system16.com)

	Taito Type X games

	Chaos Breaker / Dark Awake
	Datacarddass Dragon Ball Z
	Dinoking III
	Dinomax
	Dragon Quest Monster: Battle Road
	Gigawing Generations
	Harakari Professional Baseball
	Homura
	King Of Jurassic
	Raiden III
	Raiden IV
	Shikigami No Shiro III / The Castle of Shikigami III
	Spica Adventure
	Taisen Hot Gimmick 5
	Taisen Hot Gimmick Mix Party
	Tetris The Grand Master 3 : Terror Instinct
	The King of Fighters 98 Ultimate Match
	Trouble Witches
	Usagi Online
	Zoids Card Colosseum 

	Taito Type X+ games
	Battle Gear 4
	Battle Gear 4 Tuned
	Half Life 2 Survivor
	War Of The Grail 


	Taito Type X2 games

	Battle Fantasia
	BlazBlue: Calamity Trigger
	BlazBlue: Continuum Shift
	BlazBlue: Continuum Shift Extend
	BlazBlue: Continuum Shift II
	Chase H.Q. 2 / Chase H. Q. : Nancy Yori Kinkyuu Renraku
	D1GP Arcade
	Dariusburst AC
	Elevator Action Death Parade
	Half Life 2: Survivor Ver. 2.0
	KOF Maximum Impact: Regulation A
	KOF Maximum Impact: Regulation A2
	Matrimelee Matsuri / Power Instinct V
	Samurai Spirits Sen / Samurai Shodown: Edge of Destiny
	Street Fighter IV
	Super Street Fighter IV Arcade Edition
	The King of Fighters XII
	The King of Fighters XIII
	Wacky Races

	Taito Type X2 satellite terminal games

	Aquarian Age Alternative
	Eternal Wheel
	Lord of Vermilion


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

	MCFG_PALETTE_LENGTH(0x10000)

MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( hotgmkmp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("taito_type_x_bios.bin", 0x00, 0x10000, NO_DUMP ) // size unknown.
	/* bios, video bios etc. not dumped */

	DISK_REGION( "hdd" ) // Single 40GB drive
	DISK_IMAGE( "wdc wd400eb-11cpf0", 0, SHA1(15f8cf77b5bdc516a891022462a42521be1d7553) )
ROM_END


GAME( 2005, hotgmkmp,  0,    taito_type_x, taito_type_x, driver_device,  0, ROT0, "XNauts", "Taisen Hot Gimmick Mix Party",  GAME_NOT_WORKING | GAME_NO_SOUND )

