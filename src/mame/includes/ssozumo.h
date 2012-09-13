class ssozumo_state : public driver_device
{
public:
	ssozumo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"){ }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT8 m_sound_nmi_mask;
	DECLARE_WRITE8_MEMBER(ssozumo_sh_command_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(ssozumo_videoram_w);
	DECLARE_WRITE8_MEMBER(ssozumo_colorram_w);
	DECLARE_WRITE8_MEMBER(ssozumo_videoram2_w);
	DECLARE_WRITE8_MEMBER(ssozumo_colorram2_w);
	DECLARE_WRITE8_MEMBER(ssozumo_paletteram_w);
	DECLARE_WRITE8_MEMBER(ssozumo_scroll_w);
	DECLARE_WRITE8_MEMBER(ssozumo_flipscreen_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/ssozumo.c -----------*/




SCREEN_UPDATE_IND16( ssozumo );
