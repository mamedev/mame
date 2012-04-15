class deadang_state : public driver_device
{
public:
	deadang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_video_data(*this, "video_data"){ }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_scroll_ram;
	required_shared_ptr<UINT16> m_video_data;
	tilemap_t *m_pf3_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_layer;
	tilemap_t *m_text_layer;
	int m_deadangle_tilebank;
	int m_deadangle_oldtilebank;

	DECLARE_READ16_MEMBER(ghunter_trackball_low_r);
	DECLARE_READ16_MEMBER(ghunter_trackball_high_r);
	DECLARE_WRITE16_MEMBER(deadang_foreground_w);
	DECLARE_WRITE16_MEMBER(deadang_text_w);
	DECLARE_WRITE16_MEMBER(deadang_bank_w);
};


/*----------- defined in video/deadang.c -----------*/


VIDEO_START( deadang );
SCREEN_UPDATE_IND16( deadang );
