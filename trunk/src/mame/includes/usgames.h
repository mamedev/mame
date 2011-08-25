class usgames_state : public driver_device
{
public:
	usgames_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_charram;
	tilemap_t *m_tilemap;
};


/*----------- defined in video/usgames.c -----------*/

WRITE8_HANDLER( usgames_videoram_w );
WRITE8_HANDLER( usgames_charram_w );
VIDEO_START( usgames );
PALETTE_INIT( usgames );
SCREEN_UPDATE( usgames );
