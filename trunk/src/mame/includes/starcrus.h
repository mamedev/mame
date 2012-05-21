class starcrus_state : public driver_device
{
public:
	starcrus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	bitmap_ind16 *m_ship1_vid;
	bitmap_ind16 *m_ship2_vid;
	bitmap_ind16 *m_proj1_vid;
	bitmap_ind16 *m_proj2_vid;

	int m_s1_x;
	int m_s1_y;
	int m_s2_x;
	int m_s2_y;
	int m_p1_x;
	int m_p1_y;
	int m_p2_x;
	int m_p2_y;

	int m_p1_sprite;
	int m_p2_sprite;
	int m_s1_sprite;
	int m_s2_sprite;

	int m_engine1_on;
	int m_engine2_on;
	int m_explode1_on;
	int m_explode2_on;
	int m_launch1_on;
	int m_launch2_on;

	int m_collision_reg;

	int m_engine_sound_playing;
	int m_explode_sound_playing;
	int m_launch1_sound_playing;
	int m_launch2_sound_playing;
	DECLARE_WRITE8_MEMBER(starcrus_s1_x_w);
	DECLARE_WRITE8_MEMBER(starcrus_s1_y_w);
	DECLARE_WRITE8_MEMBER(starcrus_s2_x_w);
	DECLARE_WRITE8_MEMBER(starcrus_s2_y_w);
	DECLARE_WRITE8_MEMBER(starcrus_p1_x_w);
	DECLARE_WRITE8_MEMBER(starcrus_p1_y_w);
	DECLARE_WRITE8_MEMBER(starcrus_p2_x_w);
	DECLARE_WRITE8_MEMBER(starcrus_p2_y_w);
	DECLARE_WRITE8_MEMBER(starcrus_ship_parm_1_w);
	DECLARE_WRITE8_MEMBER(starcrus_ship_parm_2_w);
	DECLARE_WRITE8_MEMBER(starcrus_proj_parm_1_w);
	DECLARE_WRITE8_MEMBER(starcrus_proj_parm_2_w);
	DECLARE_READ8_MEMBER(starcrus_coll_det_r);
};


/*----------- defined in video/starcrus.c -----------*/

VIDEO_START( starcrus );
SCREEN_UPDATE_IND16( starcrus );
