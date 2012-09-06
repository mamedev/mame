class shisen_state : public driver_device
{
public:
	shisen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram"){ }

	int m_gfxbank;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_videoram;
	DECLARE_READ8_MEMBER(sichuan2_dsw1_r);
	DECLARE_WRITE8_MEMBER(sichuan2_coin_w);
	DECLARE_WRITE8_MEMBER(sichuan2_videoram_w);
	DECLARE_WRITE8_MEMBER(sichuan2_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sichuan2_paletteram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/shisen.c -----------*/


VIDEO_START( sichuan2 );
SCREEN_UPDATE_IND16( sichuan2 );
