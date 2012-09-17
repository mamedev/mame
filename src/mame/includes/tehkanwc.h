class tehkanwc_state : public driver_device
{
public:
	tehkanwc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"){ }

	int m_track0[2];
	int m_track1[2];
	int m_msm_data_offs;
	int m_toggle;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	UINT8 m_scroll_x[2];
	UINT8 m_led0;
	UINT8 m_led1;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(sub_cpu_halt_w);
	DECLARE_READ8_MEMBER(tehkanwc_track_0_r);
	DECLARE_READ8_MEMBER(tehkanwc_track_1_r);
	DECLARE_WRITE8_MEMBER(tehkanwc_track_0_reset_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_track_1_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound_answer_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_videoram_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_colorram_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_videoram2_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_scroll_x_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_scroll_y_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_flipscreen_x_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_flipscreen_y_w);
	DECLARE_WRITE8_MEMBER(gridiron_led0_w);
	DECLARE_WRITE8_MEMBER(gridiron_led1_w);
	DECLARE_READ8_MEMBER(tehkanwc_portA_r);
	DECLARE_READ8_MEMBER(tehkanwc_portB_r);
	DECLARE_WRITE8_MEMBER(tehkanwc_portA_w);
	DECLARE_WRITE8_MEMBER(tehkanwc_portB_w);
	DECLARE_WRITE8_MEMBER(msm_reset_w);
	DECLARE_DRIVER_INIT(teedoff);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	UINT32 screen_update_tehkanwc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
