// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Drag Race hardware

*************************************************************************/

#include "machine/watchdog.h"
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
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_position_ram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	unsigned  m_misc_flags;
	int       m_gear[2];

	/* devices */
	required_device<discrete_device> m_discrete;
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	void dragrace_misc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dragrace_misc_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dragrace_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dragrace_steering_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dragrace_scanline_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_dragrace(palette_device &palette);
	uint32_t screen_update_dragrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dragrace_frame_callback(timer_device &timer, void *ptr, int32_t param);
	void dragrace_update_misc_flags( address_space &space );
};

/*----------- defined in audio/dragrace.c -----------*/
DISCRETE_SOUND_EXTERN( dragrace );
