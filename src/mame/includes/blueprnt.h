/***************************************************************************

    Blue Print

***************************************************************************/

class blueprnt_state : public driver_device
{
public:
	blueprnt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_colorram;
	UINT8 * m_spriteram;
	UINT8 * m_scrollram;
	size_t  m_spriteram_size;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_gfx_bank;

	/* misc */
	int     m_dipsw;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/blueprnt.c -----------*/

WRITE8_HANDLER( blueprnt_videoram_w );
WRITE8_HANDLER( blueprnt_colorram_w );
WRITE8_HANDLER( blueprnt_flipscreen_w );

PALETTE_INIT( blueprnt );
VIDEO_START( blueprnt );
SCREEN_UPDATE( blueprnt );
