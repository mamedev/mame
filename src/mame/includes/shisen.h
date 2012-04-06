class shisen_state : public driver_device
{
public:
	shisen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_gfxbank;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_paletteram;
	UINT8 *m_videoram;
	DECLARE_READ8_MEMBER(sichuan2_dsw1_r);
	DECLARE_WRITE8_MEMBER(sichuan2_coin_w);
	DECLARE_WRITE8_MEMBER(sichuan2_videoram_w);
	DECLARE_WRITE8_MEMBER(sichuan2_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sichuan2_paletteram_w);
};


/*----------- defined in video/shisen.c -----------*/


VIDEO_START( sichuan2 );
SCREEN_UPDATE_IND16( sichuan2 );
