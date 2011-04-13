/*************************************************************************

    empty.c

    Empty driver.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************/

#include "emu.h"
#include "render.h"
#include "uimenu.h"


/*************************************
 *
 *  Machine "start"
 *
 *************************************/

static MACHINE_START( ___empty )
{
	/* force the UI to show the game select screen */
	ui_menu_force_game_select(machine, &machine.render().ui_container());
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( ___empty, driver_device )

	MCFG_MACHINE_START(___empty)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640,480)
	MCFG_SCREEN_VISIBLE_AREA(0,639, 0,479)
	MCFG_SCREEN_REFRESH_RATE(30)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ___empty )
	ROM_REGION( 0x10, "user1", ROMREGION_ERASEFF )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2007, ___empty, 0, ___empty, 0, 0, ROT0, "MAME", "No Driver Loaded", GAME_NO_SOUND )
