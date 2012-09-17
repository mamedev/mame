class m90_state : public driver_device
{
public:
	m90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_data(*this, "video_data"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT16> m_video_data;
	optional_shared_ptr<UINT16> m_spriteram;
	UINT16 m_video_control_data[8];
	tilemap_t *m_pf1_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_wide_layer;
	tilemap_t *m_pf2_wide_layer;
	UINT8 m_last_pf1;
	UINT8 m_last_pf2;
	device_t *m_audio;
	DECLARE_WRITE16_MEMBER(m90_coincounter_w);
	DECLARE_WRITE16_MEMBER(quizf1_bankswitch_w);
	DECLARE_WRITE16_MEMBER(dynablsb_sound_command_w);
	DECLARE_WRITE16_MEMBER(unknown_w);
	DECLARE_WRITE16_MEMBER(m90_video_control_w);
	DECLARE_WRITE16_MEMBER(m90_video_w);
	DECLARE_DRIVER_INIT(bomblord);
	DECLARE_DRIVER_INIT(quizf1);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1w_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2w_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf1w_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf2w_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf1w_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf2w_tile_info);
	virtual void machine_start();
	virtual void video_start();
	DECLARE_VIDEO_START(bomblord);
	DECLARE_VIDEO_START(dynablsb);
	UINT32 screen_update_m90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bomblord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dynablsb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/m90.c -----------*/







