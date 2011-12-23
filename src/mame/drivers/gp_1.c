/*
    Game Plan MPU-1
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class gp_1_state : public driver_device
{
public:
	gp_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( gp_1_map, AS_PROGRAM, 8, gp_1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( gp_1 )
INPUT_PORTS_END

void gp_1_state::machine_reset()
{
}

static DRIVER_INIT( gp_1 )
{
}

static MACHINE_CONFIG_START( gp_1, gp_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2457600)
	MCFG_CPU_PROGRAM_MAP(gp_1_map)
MACHINE_CONFIG_END


ROM_START( gp_110 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "a-110.u12", 0x0000, 0x0800, CRC(ed0d518b) SHA1(8f3ca8792ad907c660d9149a1aa3a3528c7573e3))
	ROM_LOAD( "b1-110.u13", 0x0800, 0x0800, CRC(a223f2e8) SHA1(767e15e19e11399935c890c1d1034dccf1ad7f92))
ROM_END

GAME(1978,	gp_110,	0,	gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",	"Model 110",	GAME_IS_BIOS_ROOT)

/*-------------------------------------------------------------------
/ Black Velvet (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_blvelvet    rom_gp_110
/*-------------------------------------------------------------------
/ Camel Lights (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_camlight    rom_gp_110
/*-------------------------------------------------------------------
/ Chuck-A-Luck (October 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_chucklck    rom_gp_110
/*-------------------------------------------------------------------
/ Family Fun! (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(famlyfun)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "family.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "family.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

/*-------------------------------------------------------------------
/ Foxy Lady (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_foxylady    rom_gp_110
/*-------------------------------------------------------------------
/ Real (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_real    rom_gp_110
/*-------------------------------------------------------------------
/ Rio (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_rio    rom_gp_110
/*-------------------------------------------------------------------
/ Star Trip (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(startrip)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "startrip.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "startrip.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

GAME(1978,	blvelvet,	gp_110,		gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Black Velvet",		GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	camlight,	gp_110,		gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Camel Lights",		GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	chucklck,	gp_110,		gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Chuck-A-Luck",		GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	famlyfun,	0,			gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Family Fun!",		GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	foxylady,	gp_110,		gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Foxy Lady",		GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	real,		gp_110,		gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Real",				GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	rio,		gp_110,		gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Rio",				GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	startrip,	0,			gp_1,	gp_1,	gp_1,	ROT0,	"Game Plan",				"Star Trip",		GAME_IS_SKELETON_MECHANICAL)
