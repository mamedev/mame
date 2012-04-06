class seicross_state : public driver_device
{
public:
	seicross_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_nvram;
	size_t m_nvram_size;

	UINT8 m_portb;

	size_t m_spriteram_size;
	size_t m_spriteram2_size;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_row_scroll;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(seicross_videoram_w);
	DECLARE_WRITE8_MEMBER(seicross_colorram_w);
};


/*----------- defined in video/seicross.c -----------*/


PALETTE_INIT( seicross );
VIDEO_START( seicross );
SCREEN_UPDATE_IND16( seicross );
