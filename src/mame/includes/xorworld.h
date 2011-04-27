class xorworld_state : public driver_device
{
public:
	xorworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	tilemap_t *m_bg_tilemap;
	UINT16 *m_spriteram;
};


/*----------- defined in video/xorworld.c -----------*/

WRITE16_HANDLER( xorworld_videoram16_w );

PALETTE_INIT( xorworld );
VIDEO_START( xorworld );
SCREEN_UPDATE( xorworld );
