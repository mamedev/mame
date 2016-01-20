// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#include "sound/samples.h"
class starcrus_state : public driver_device
{
public:
	starcrus_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	std::unique_ptr<bitmap_ind16> m_ship1_vid;
	std::unique_ptr<bitmap_ind16> m_ship2_vid;
	std::unique_ptr<bitmap_ind16> m_proj1_vid;
	std::unique_ptr<bitmap_ind16> m_proj2_vid;

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

	DECLARE_WRITE8_MEMBER(s1_x_w);
	DECLARE_WRITE8_MEMBER(s1_y_w);
	DECLARE_WRITE8_MEMBER(s2_x_w);
	DECLARE_WRITE8_MEMBER(s2_y_w);
	DECLARE_WRITE8_MEMBER(p1_x_w);
	DECLARE_WRITE8_MEMBER(p1_y_w);
	DECLARE_WRITE8_MEMBER(p2_x_w);
	DECLARE_WRITE8_MEMBER(p2_y_w);
	DECLARE_WRITE8_MEMBER(ship_parm_1_w);
	DECLARE_WRITE8_MEMBER(ship_parm_2_w);
	DECLARE_WRITE8_MEMBER(proj_parm_1_w);
	DECLARE_WRITE8_MEMBER(proj_parm_2_w);
	DECLARE_READ8_MEMBER(coll_det_r);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int collision_check_s1s2();
	int collision_check_p1p2();
	int collision_check_s1p1p2();
	int collision_check_s2p1p2();
};
