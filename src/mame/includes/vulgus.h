class vulgus_state : public driver_device
{
public:
	vulgus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_scroll_low(*this, "scroll_low"),
		m_scroll_high(*this, "scroll_high"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"){ }

	required_shared_ptr<UINT8> m_scroll_low;
	required_shared_ptr<UINT8> m_scroll_high;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	int m_palette_bank;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(vulgus_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(vulgus_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(vulgus_c804_w);
	DECLARE_WRITE8_MEMBER(vulgus_palette_bank_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/vulgus.c -----------*/




SCREEN_UPDATE_IND16( vulgus );
