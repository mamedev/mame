/***************************************************************************

  famibox.c
  Preliminary driver file to handle emulation of the
  Nintendo FamicomBox

In 1986, Nintendo Co., Ltd. began distrubuting the FamicomBox (SSS-CDS) which
allowed gamers to test out and play up to 15 different Famicom games which
could be installed in the unit itself.  Like it's sequel, the Super Famicom Box,
it was found in hotels often set up to accept 100 yen coins giving you an
adjusted amount of gametime (10 or 20 minutes - DIP selectable).  Sharp also
produced a version of the FamicomBox called FamicomStation which was more
of a consumer (non-coin accept) unit.  Besides that, the equipment and
capabilities between the two are thought to be virtually idential - save the
case style and cartridge/case color:  (FamicomBox = Black, FamicomStation = Gray).



Specific Hardware information
-----------------------------
The unit had 3 controllers - 2 standard NES controllers and a NES Zapper light
gun.  The cartridges are shaped and appear to be idential to NES 72-pin
cartridges.  Unfortunately, it was made to play only the games specifically
released for it.  Why?

- The FamicomBox will not run mmc3 games and many other advanced mappers
- There a special lockout chip, but the lockout chip connects to different pins on
  a FamicomBox cartridge?s connector than a regular cart
- The lockout chips in the system and the games have to ?talk? before the system will
  load any games into its menu.

Here?s a list of some of the games known to have come with the FamicomBox:
1943; Baseball; Bomber Man; Devil World; Donkey Kong; Donkey Kong Jr.; Duck Hunt;
Excite Bike; F1 Race; Fighting Golf; Golf; Gradius; Hogan?s Alley; Ice Climbers;
Ice Hockey; Knight Rider; Makaimura: Ghosts ?n Goblins; McKids; Mah-Jong; Mario Bros.;
Mike Tyson?s Punch-Out!!; Ninja Ryukenden; Operation Wolf (?); Punch-Out!!; Rock Man;
Rygar; Senjou no Ookami; Soccer League Winner?s Cup; Super Chinese 2; Super Mario Bros;
Tag Team Pro Wrestling; Takahashi Meijin no Boukenjima; Tennis; Twin Bee;
Volleyball; Wild Gunman; Wrecking Crew.

Here?s a list of some of the games known to have come with the FamicomStation:
1943; Baseball; Donkey Kong; Duck Hunt; F1 Race; Golf; Kame no Ongaeshi:
Urashima Densetsu; Mah-Jong; Mario Bros.; Night Raider; Senjou no Ookami;
Soccer League Winner?s Cup; Super Chinese 2; Super Mario Bros; Tag Team Pro Wrestling;
Takahashi Meijin no Boukenjima; Tennis; Wild Gunman; Wrecking Crew.

***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cartslot.h"
#include "sound/nes_apu.h"

class famibox_state : public driver_device
{
public:
	famibox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( famibox_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( famibox )
INPUT_PORTS_END

static MACHINE_RESET( famibox )
{
}

//static DRIVER_INIT( famibox )
//{
//}

static MACHINE_CONFIG_START( famibox, famibox_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", N2A03, N2A03_DEFAULTCLOCK)
	MCFG_CPU_PROGRAM_MAP(famibox_map)

	MCFG_MACHINE_RESET( famibox )
	MCFG_PALETTE_LENGTH(4*16*8)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ FamicomBox
/-------------------------------------------------------------------*/
ROM_START(famibox)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sss_menu.prg", 0x8000, 0x8000, CRC(da1eb8d2) SHA1(943e3b0edfbf9bd3ee87dc5f298621b9ddc98db8))
	ROM_LOAD("sss_menu.chr", 0x2000, 0x2000, CRC(a43d4435) SHA1(ee56b4d2110aff394bf2c8cd3414ca175ace01bd))
ROM_END

GAME( 1986,  famibox,      0,  famibox,  famibox,  0, ROT0, "Nintendo", "FamicomBox", GAME_NOT_WORKING | GAME_NO_SOUND)

