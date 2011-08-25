class gatron_state : public driver_device
{
public:
	gatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/gatron.c -----------*/

WRITE8_HANDLER( gat_videoram_w );
PALETTE_INIT( gat );
VIDEO_START( gat );
SCREEN_UPDATE( gat );

