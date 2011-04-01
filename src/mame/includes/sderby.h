class sderby_state : public driver_device
{
public:
	sderby_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_spriteram;
	size_t m_spriteram_size;

	UINT16 *m_videoram;
	UINT16 *m_md_videoram;
	UINT16 *m_fg_videoram;

	tilemap_t *m_tilemap;
	tilemap_t *m_md_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT16 m_scroll[6];
};


/*----------- defined in video/sderby.c -----------*/

WRITE16_HANDLER( sderby_videoram_w );
WRITE16_HANDLER( sderby_md_videoram_w );
WRITE16_HANDLER( sderby_fg_videoram_w );
VIDEO_START( sderby );
SCREEN_UPDATE( sderby );
SCREEN_UPDATE( pmroulet );
WRITE16_HANDLER( sderby_scroll_w );
