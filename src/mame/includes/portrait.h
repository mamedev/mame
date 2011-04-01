class portrait_state : public driver_device
{
public:
	portrait_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_bgvideoram;
	UINT8 *m_fgvideoram;
	int m_scroll;
	tilemap_t *m_foreground;
	tilemap_t *m_background;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/portrait.c -----------*/

PALETTE_INIT( portrait );
VIDEO_START( portrait );
SCREEN_UPDATE( portrait );
WRITE8_HANDLER( portrait_bgvideo_write );
WRITE8_HANDLER( portrait_fgvideo_write );
