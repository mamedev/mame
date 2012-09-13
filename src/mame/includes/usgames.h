class usgames_state : public driver_device
{
public:
	usgames_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_charram;
	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(usgames_rombank_w);
	DECLARE_WRITE8_MEMBER(lamps1_w);
	DECLARE_WRITE8_MEMBER(lamps2_w);
	DECLARE_WRITE8_MEMBER(usgames_videoram_w);
	DECLARE_WRITE8_MEMBER(usgames_charram_w);
	TILE_GET_INFO_MEMBER(get_usgames_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/usgames.c -----------*/



SCREEN_UPDATE_IND16( usgames );
