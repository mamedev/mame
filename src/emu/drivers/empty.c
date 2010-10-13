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

static MACHINE_START( empty )
{
	/* force the UI to show the game select screen */
	ui_menu_force_game_select(machine, &machine->render().ui_container());
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( empty, driver_device )

	MDRV_MACHINE_START(empty)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640,480)
	MDRV_SCREEN_VISIBLE_AREA(0,639, 0,479)
	MDRV_SCREEN_REFRESH_RATE(30)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( empty )
	ROM_REGION( 0x10, "user1", 0 )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2007, empty, 0, empty, 0, 0, ROT0, "MAME", "No Driver Loaded", 0 )
