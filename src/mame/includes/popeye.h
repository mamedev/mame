class popeye_state : public driver_device
{
public:
	popeye_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_background_pos(*this, "background_pos"),
		m_palettebank(*this, "palettebank"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	UINT8 m_prot0;
	UINT8 m_prot1;
	UINT8 m_prot_shift;
	int m_dswbit;
	required_shared_ptr<UINT8> m_background_pos;
	required_shared_ptr<UINT8> m_palettebank;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	UINT8 *m_bitmapram;
	bitmap_ind16 *m_tmpbitmap2;
	UINT8 m_invertmask;
	UINT8 m_bitmap_type;
	tilemap_t *m_fg_tilemap;
	UINT8 m_lastflip;

	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(popeye_videoram_w);
	DECLARE_WRITE8_MEMBER(popeye_colorram_w);
	DECLARE_WRITE8_MEMBER(popeye_bitmap_w);
	DECLARE_WRITE8_MEMBER(skyskipr_bitmap_w);
	DECLARE_WRITE8_MEMBER(popeye_portB_w);
	DECLARE_READ8_MEMBER(popeye_portA_r);
	DECLARE_DRIVER_INIT(skyskipr);
	DECLARE_DRIVER_INIT(popeye);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(popeye);
	DECLARE_PALETTE_INIT(popeyebl);
};


/*----------- defined in video/popeye.c -----------*/






SCREEN_UPDATE_IND16( popeye );
