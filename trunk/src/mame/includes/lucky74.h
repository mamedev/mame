class lucky74_state : public driver_device
{
public:
	lucky74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_ym2149_portb;
	UINT8 m_usart_8251;
	UINT8 m_copro_sm7831;
	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	UINT8 m_adpcm_reg[6];
	UINT8 m_adpcm_busy_line;
	UINT8 *m_fg_videoram;
	UINT8 *m_fg_colorram;
	UINT8 *m_bg_videoram;
	UINT8 *m_bg_colorram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/lucky74.c -----------*/

WRITE8_HANDLER( lucky74_fg_videoram_w );
WRITE8_HANDLER( lucky74_fg_colorram_w );
WRITE8_HANDLER( lucky74_bg_videoram_w );
WRITE8_HANDLER( lucky74_bg_colorram_w );
PALETTE_INIT( lucky74 );
VIDEO_START( lucky74 );
SCREEN_UPDATE( lucky74 );
