/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/

class zaxxon_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, zaxxon_state(machine)); }

	zaxxon_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT8 *colorram;
	UINT8 *videoram;
	UINT8 *spriteram;

	UINT8 int_enabled;
	UINT8 coin_status[3];
	UINT8 coin_enable[3];

	UINT8 razmataz_dial_pos[2];
	UINT16 razmataz_counter;

	UINT8 sound_state[3];
	UINT8 bg_enable;
	UINT8 bg_color;
	UINT16 bg_position;
	UINT8 fg_color;

	UINT8 congo_fg_bank;
	UINT8 congo_color_bank;
	UINT8 congo_custom[4];

	const UINT8 *color_codes;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
};


/*----------- defined in audio/zaxxon.c -----------*/

WRITE8_DEVICE_HANDLER( zaxxon_sound_a_w );
WRITE8_DEVICE_HANDLER( zaxxon_sound_b_w );
WRITE8_DEVICE_HANDLER( zaxxon_sound_c_w );

WRITE8_DEVICE_HANDLER( congo_sound_b_w );
WRITE8_DEVICE_HANDLER( congo_sound_c_w );

MACHINE_DRIVER_EXTERN( zaxxon_samples );
MACHINE_DRIVER_EXTERN( congo_samples );


/*----------- defined in video/zaxxon.c -----------*/

WRITE8_HANDLER( zaxxon_flipscreen_w );
WRITE8_HANDLER( zaxxon_fg_color_w );
WRITE8_HANDLER( zaxxon_bg_position_w );
WRITE8_HANDLER( zaxxon_bg_color_w );
WRITE8_HANDLER( zaxxon_bg_enable_w );

WRITE8_HANDLER( zaxxon_videoram_w );
WRITE8_HANDLER( congo_colorram_w );

WRITE8_HANDLER( congo_fg_bank_w );
WRITE8_HANDLER( congo_color_bank_w );
WRITE8_HANDLER( congo_sprite_custom_w );

PALETTE_INIT( zaxxon );

VIDEO_START( zaxxon );
VIDEO_START( razmataz );
VIDEO_START( congo );

VIDEO_UPDATE( zaxxon );
VIDEO_UPDATE( razmataz );
VIDEO_UPDATE( congo );
VIDEO_UPDATE( futspy );
