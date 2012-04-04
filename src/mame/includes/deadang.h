class deadang_state : public driver_device
{
public:
	deadang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 *m_video_data;
	UINT16 *m_scroll_ram;
	tilemap_t *m_pf3_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_layer;
	tilemap_t *m_text_layer;
	int m_deadangle_tilebank;
	int m_deadangle_oldtilebank;
	UINT16 *m_spriteram;
	DECLARE_READ16_MEMBER(ghunter_trackball_low_r);
	DECLARE_READ16_MEMBER(ghunter_trackball_high_r);
};


/*----------- defined in video/deadang.c -----------*/

WRITE16_HANDLER( deadang_foreground_w );
WRITE16_HANDLER( deadang_text_w );
WRITE16_HANDLER( deadang_bank_w );

VIDEO_START( deadang );
SCREEN_UPDATE_IND16( deadang );
