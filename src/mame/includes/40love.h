class fortyl_state : public driver_device
{
public:
	fortyl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_video_ctrl(*this, "video_ctrl"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_spriteram2(*this, "spriteram2"),
		m_mcu_ram(*this, "mcu_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_video_ctrl;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram2;
	optional_shared_ptr<UINT8> m_mcu_ram;

	/* video-related */
	bitmap_ind16    *m_tmp_bitmap1;
	bitmap_ind16    *m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	UINT8       m_flipscreen;
	UINT8		m_pix_redraw;
	UINT8       m_xoffset;
	UINT8       *m_pixram1;
	UINT8       *m_pixram2;
	bitmap_ind16    *m_pixel_bitmap1;
	bitmap_ind16    *m_pixel_bitmap2;
	int         m_pixram_sel;

	/* sound-related */
	int         m_sound_nmi_enable;
	int			m_pending_nmi;

	/* fake mcu */
	UINT8       m_from_mcu;
	int         m_mcu_sent;
	int			m_main_sent;
	UINT8       m_mcu_in[2][16];
	UINT8		m_mcu_out[2][16];
	int         m_mcu_cmd;

	/* misc */
	int         m_pix_color[4];
	UINT8       m_pix1;
	UINT8		m_pix2[2];
	UINT8       m_snd_data;
	UINT8		m_snd_flag;
	int         m_vol_ctrl[16];
	UINT8       m_snd_ctrl0;
	UINT8		m_snd_ctrl1;
	UINT8		m_snd_ctrl2;
	UINT8		m_snd_ctrl3;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(fortyl_coin_counter_w);
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(pix1_w);
	DECLARE_WRITE8_MEMBER(pix2_w);
	DECLARE_READ8_MEMBER(pix1_r);
	DECLARE_READ8_MEMBER(pix2_r);
	DECLARE_WRITE8_MEMBER(undoukai_mcu_w);
	DECLARE_READ8_MEMBER(undoukai_mcu_r);
	DECLARE_READ8_MEMBER(undoukai_mcu_status_r);
	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_READ8_MEMBER(snd_flag_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE8_MEMBER(fortyl_pixram_sel_w);
	DECLARE_READ8_MEMBER(fortyl_pixram_r);
	DECLARE_WRITE8_MEMBER(fortyl_pixram_w);
	DECLARE_WRITE8_MEMBER(fortyl_bg_videoram_w);
	DECLARE_READ8_MEMBER(fortyl_bg_videoram_r);
	DECLARE_WRITE8_MEMBER(fortyl_bg_colorram_w);
	DECLARE_READ8_MEMBER(fortyl_bg_colorram_r);
	DECLARE_WRITE8_MEMBER(pix1_mcu_w);
	DECLARE_WRITE8_MEMBER(sound_control_0_w);
	DECLARE_WRITE8_MEMBER(sound_control_1_w);
	DECLARE_WRITE8_MEMBER(sound_control_2_w);
	DECLARE_WRITE8_MEMBER(sound_control_3_w);
	DECLARE_DRIVER_INIT(undoukai);
	DECLARE_DRIVER_INIT(40love);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(40love);
	DECLARE_MACHINE_RESET(40love);
	DECLARE_MACHINE_START(undoukai);
	DECLARE_MACHINE_RESET(undoukai);
	DECLARE_MACHINE_RESET(common);
	DECLARE_MACHINE_RESET(ta7630);
};


/*----------- defined in video/40love.c -----------*/



SCREEN_UPDATE_IND16( fortyl );

