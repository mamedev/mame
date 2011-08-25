/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/

class zaxxon_state : public driver_device
{
public:
	zaxxon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_colorram;
	UINT8 *m_videoram;
	UINT8 *m_spriteram;

	UINT8 m_int_enabled;
	UINT8 m_coin_status[3];
	UINT8 m_coin_enable[3];

	UINT8 m_razmataz_dial_pos[2];
	UINT16 m_razmataz_counter;

	UINT8 m_sound_state[3];
	UINT8 m_bg_enable;
	UINT8 m_bg_color;
	UINT16 m_bg_position;
	UINT8 m_fg_color;

	UINT8 m_congo_fg_bank;
	UINT8 m_congo_color_bank;
	UINT8 m_congo_custom[4];

	const UINT8 *m_color_codes;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in audio/zaxxon.c -----------*/

WRITE8_DEVICE_HANDLER( zaxxon_sound_a_w );
WRITE8_DEVICE_HANDLER( zaxxon_sound_b_w );
WRITE8_DEVICE_HANDLER( zaxxon_sound_c_w );

WRITE8_DEVICE_HANDLER( congo_sound_b_w );
WRITE8_DEVICE_HANDLER( congo_sound_c_w );

MACHINE_CONFIG_EXTERN( zaxxon_samples );
MACHINE_CONFIG_EXTERN( congo_samples );


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

SCREEN_UPDATE( zaxxon );
SCREEN_UPDATE( razmataz );
SCREEN_UPDATE( congo );
SCREEN_UPDATE( futspy );
