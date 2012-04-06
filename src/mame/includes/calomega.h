class calomega_state : public driver_device
{
public:
	calomega_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_tx_line;
	UINT8 m_rx_line;
	int m_s903_mux_data;
	int m_s905_mux_data;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(calomega_videoram_w);
	DECLARE_WRITE8_MEMBER(calomega_colorram_w);
};


/*----------- defined in video/calomega.c -----------*/

PALETTE_INIT( calomega );
VIDEO_START( calomega );
SCREEN_UPDATE_IND16( calomega );
