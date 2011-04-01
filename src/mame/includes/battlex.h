/***************************************************************************

    Battle Cross

***************************************************************************/

class battlex_state : public driver_device
{
public:
	battlex_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_in0_b4;

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	UINT8 m_scroll_lsb;
	UINT8 m_scroll_msb;
	UINT8 m_starfield_enabled;
};


/*----------- defined in video/battlex.c -----------*/

extern WRITE8_HANDLER( battlex_palette_w );
extern WRITE8_HANDLER( battlex_videoram_w );
extern WRITE8_HANDLER( battlex_scroll_x_lsb_w );
extern WRITE8_HANDLER( battlex_scroll_x_msb_w );
extern WRITE8_HANDLER( battlex_scroll_starfield_w );
extern WRITE8_HANDLER( battlex_flipscreen_w );

extern VIDEO_START( battlex );
extern SCREEN_UPDATE( battlex );
