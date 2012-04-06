class wolfpack_state : public driver_device
{
public:
	wolfpack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_collision;
	UINT8* m_alpha_num_ram;
	unsigned m_current_index;
	UINT8 m_video_invert;
	UINT8 m_ship_reflect;
	UINT8 m_pt_pos_select;
	UINT8 m_pt_horz;
	UINT8 m_pt_pic;
	UINT8 m_ship_h;
	UINT8 m_torpedo_pic;
	UINT8 m_ship_size;
	UINT8 m_ship_h_precess;
	UINT8 m_ship_pic;
	UINT8 m_torpedo_h;
	UINT8 m_torpedo_v;
	UINT8* m_LFSR;
	bitmap_ind16 m_helper;
	DECLARE_READ8_MEMBER(wolfpack_misc_r);
	DECLARE_WRITE8_MEMBER(wolfpack_high_explo_w);
	DECLARE_WRITE8_MEMBER(wolfpack_sonar_ping_w);
	DECLARE_WRITE8_MEMBER(wolfpack_sirlat_w);
	DECLARE_WRITE8_MEMBER(wolfpack_pt_sound_w);
	DECLARE_WRITE8_MEMBER(wolfpack_launch_torpedo_w);
	DECLARE_WRITE8_MEMBER(wolfpack_low_explo_w);
	DECLARE_WRITE8_MEMBER(wolfpack_screw_cont_w);
	DECLARE_WRITE8_MEMBER(wolfpack_lamp_flash_w);
	DECLARE_WRITE8_MEMBER(wolfpack_warning_light_w);
	DECLARE_WRITE8_MEMBER(wolfpack_audamp_w);
	DECLARE_WRITE8_MEMBER(wolfpack_attract_w);
	DECLARE_WRITE8_MEMBER(wolfpack_credit_w);
	DECLARE_WRITE8_MEMBER(wolfpack_coldetres_w);
	DECLARE_WRITE8_MEMBER(wolfpack_ship_size_w);
	DECLARE_WRITE8_MEMBER(wolfpack_video_invert_w);
	DECLARE_WRITE8_MEMBER(wolfpack_ship_reflect_w);
	DECLARE_WRITE8_MEMBER(wolfpack_pt_pos_select_w);
	DECLARE_WRITE8_MEMBER(wolfpack_pt_horz_w);
	DECLARE_WRITE8_MEMBER(wolfpack_pt_pic_w);
	DECLARE_WRITE8_MEMBER(wolfpack_ship_h_w);
	DECLARE_WRITE8_MEMBER(wolfpack_torpedo_pic_w);
	DECLARE_WRITE8_MEMBER(wolfpack_ship_h_precess_w);
	DECLARE_WRITE8_MEMBER(wolfpack_ship_pic_w);
	DECLARE_WRITE8_MEMBER(wolfpack_torpedo_h_w);
	DECLARE_WRITE8_MEMBER(wolfpack_torpedo_v_w);
};


/*----------- defined in video/wolfpack.c -----------*/

PALETTE_INIT( wolfpack );
SCREEN_UPDATE_IND16( wolfpack );
VIDEO_START( wolfpack );
SCREEN_VBLANK( wolfpack );

