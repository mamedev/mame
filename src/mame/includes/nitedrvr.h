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
	nitedrvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_hvc(*this, "hvc"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"){ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hvc;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* input */
	uint8_t m_gear;
	uint8_t m_track;
	int32_t m_steering_buf;
	int32_t m_steering_val;
	uint8_t m_crash_en;
	uint8_t m_crash_data;
	uint8_t m_crash_data_en;  // IC D8
	uint8_t m_ac_line;
	int32_t m_last_steering_val;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t nitedrvr_steering_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nitedrvr_steering_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nitedrvr_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nitedrvr_in1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nitedrvr_out0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nitedrvr_out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nitedrvr_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_nitedrvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nitedrvr_crash_toggle_callback(timer_device &timer, void *ptr, int32_t param);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey);
	void draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nitedrvr_steering();
};

/*----------- defined in audio/nitedrvr.c -----------*/
DISCRETE_SOUND_EXTERN( nitedrvr );
