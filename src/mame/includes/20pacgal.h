/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/


typedef struct __20pacgal_state _20pacgal_state;
struct __20pacgal_state
{
	/* memory pointers */
	UINT8 *char_gfx_ram;
	UINT8 *sprite_gfx_ram;
	UINT8 *video_ram;
	UINT8 *sprite_ram;
	UINT8 *sprite_color_lookup;
	UINT8 *flip;

	/* machine state */
	UINT8 game_selected;	/* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	running_device *maincpu;
	running_device *eeprom;
};



/*----------- defined in video/20pacgal.c -----------*/

MACHINE_DRIVER_EXTERN( 20pacgal_video );
