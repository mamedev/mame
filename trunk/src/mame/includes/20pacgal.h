/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/


class _20pacgal_state : public driver_device
{
public:
	_20pacgal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *m_char_gfx_ram;
	UINT8 *m_video_ram;
	UINT8 *m_flip;
	UINT8 *m_stars_seed;
	UINT8 *m_stars_ctrl;

	/* machine state */
	UINT8 m_game_selected;	/* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	device_t *m_maincpu;
	device_t *m_eeprom;

	/* memory */
	UINT8 m_sprite_gfx_ram[0x2000];
	UINT8 m_sprite_ram[0x180];
	UINT8 m_sprite_color_lookup[0x100];
	UINT8 m_ram_48000[0x2000];

	/* 25pacman and 20pacgal store the sprite palette at a different address, this is a hardware difference and confirmed NOT to be a register */
	UINT8 m_sprite_pal_base;
};



/*----------- defined in video/20pacgal.c -----------*/

MACHINE_CONFIG_EXTERN( 20pacgal_video );
