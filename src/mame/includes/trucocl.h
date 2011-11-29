class trucocl_state : public driver_device
{
public:
	trucocl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_cur_dac_address;
	int m_cur_dac_address_index;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;

	UINT8 m_irq_mask;
};


/*----------- defined in video/trucocl.c -----------*/

WRITE8_HANDLER( trucocl_videoram_w );
WRITE8_HANDLER( trucocl_colorram_w );
PALETTE_INIT( trucocl );
VIDEO_START( trucocl );
SCREEN_UPDATE( trucocl );
