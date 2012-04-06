class gatron_state : public driver_device
{
public:
	gatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(output_port_0_w);
	DECLARE_WRITE8_MEMBER(gat_videoram_w);
};


/*----------- defined in video/gatron.c -----------*/

PALETTE_INIT( gat );
VIDEO_START( gat );
SCREEN_UPDATE_IND16( gat );

