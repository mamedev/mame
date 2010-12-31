/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/


class _20pacgal_state : public driver_device
{
public:
	_20pacgal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *char_gfx_ram;
	UINT8 sprite_gfx_ram[0x2000];
	UINT8 *video_ram;
	UINT8 sprite_ram[0x180];
	UINT8 sprite_color_lookup[0x100];
	UINT8 *flip;
	UINT8 *stars_seed;
	UINT8 *stars_ctrl;

	/* machine state */
	UINT8 game_selected;	/* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	device_t *maincpu;
	device_t *eeprom;

	/* bank support */
	UINT8 *ram_48000;

	/* 25pacman and 20pacgal store the sprite palette at a different address, this is a hardware difference and confirmed NOT to be a register */
	UINT8 sprite_pal_base;
};



/*----------- defined in video/20pacgal.c -----------*/

MACHINE_CONFIG_EXTERN( 20pacgal_video );
