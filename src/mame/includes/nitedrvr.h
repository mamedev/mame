// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Night Driver hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define NITEDRVR_BANG_DATA  NODE_01
#define NITEDRVR_SKID1_EN   NODE_02
#define NITEDRVR_SKID2_EN   NODE_03
#define NITEDRVR_MOTOR_DATA NODE_04
#define NITEDRVR_CRASH_EN   NODE_05
#define NITEDRVR_ATTRACT_EN NODE_06


class nitedrvr_state : public driver_device
{
public:
	nitedrvr_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_hvc(*this, "hvc"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_hvc;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* input */
	UINT8 m_gear;
	UINT8 m_track;
	INT32 m_steering_buf;
	INT32 m_steering_val;
	UINT8 m_crash_en;
	UINT8 m_crash_data;
	UINT8 m_crash_data_en;  // IC D8
	UINT8 m_ac_line;
	INT32 m_last_steering_val;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(nitedrvr_steering_reset_r);
	DECLARE_WRITE8_MEMBER(nitedrvr_steering_reset_w);
	DECLARE_READ8_MEMBER(nitedrvr_in0_r);
	DECLARE_READ8_MEMBER(nitedrvr_in1_r);
	DECLARE_WRITE8_MEMBER(nitedrvr_out0_w);
	DECLARE_WRITE8_MEMBER(nitedrvr_out1_w);
	DECLARE_WRITE8_MEMBER(nitedrvr_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_nitedrvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(nitedrvr_crash_toggle_callback);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey);
	void draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nitedrvr_steering();
};

/*----------- defined in audio/nitedrvr.c -----------*/
DISCRETE_SOUND_EXTERN( nitedrvr );
