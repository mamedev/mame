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
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(trucocl_videoram_w);
	DECLARE_WRITE8_MEMBER(trucocl_colorram_w);
};


/*----------- defined in video/trucocl.c -----------*/

PALETTE_INIT( trucocl );
VIDEO_START( trucocl );
SCREEN_UPDATE_IND16( trucocl );
