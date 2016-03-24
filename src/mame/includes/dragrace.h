// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Drag Race hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define DRAGRACE_SCREECH1_EN    NODE_01
#define DRAGRACE_SCREECH2_EN    NODE_02
#define DRAGRACE_LOTONE_EN      NODE_03
#define DRAGRACE_HITONE_EN      NODE_04
#define DRAGRACE_EXPLODE1_EN    NODE_05
#define DRAGRACE_EXPLODE2_EN    NODE_06
#define DRAGRACE_MOTOR1_DATA    NODE_07
#define DRAGRACE_MOTOR2_DATA    NODE_08
#define DRAGRACE_MOTOR1_EN      NODE_80
#define DRAGRACE_MOTOR2_EN      NODE_81
#define DRAGRACE_KLEXPL1_EN     NODE_82
#define DRAGRACE_KLEXPL2_EN     NODE_83
#define DRAGRACE_ATTRACT_EN     NODE_09


class dragrace_state : public driver_device
{
public:
	dragrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_position_ram(*this, "position_ram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_position_ram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	unsigned  m_misc_flags;
	int       m_gear[2];

	/* devices */
	required_device<discrete_device> m_discrete;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	DECLARE_WRITE8_MEMBER(dragrace_misc_w);
	DECLARE_WRITE8_MEMBER(dragrace_misc_clear_w);
	DECLARE_READ8_MEMBER(dragrace_input_r);
	DECLARE_READ8_MEMBER(dragrace_steering_r);
	DECLARE_READ8_MEMBER(dragrace_scanline_r);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(dragrace);
	UINT32 screen_update_dragrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(dragrace_frame_callback);
	void dragrace_update_misc_flags( address_space &space );
};

/*----------- defined in audio/dragrace.c -----------*/
DISCRETE_SOUND_EXTERN( dragrace );
