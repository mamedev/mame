// license:BSD-3-Clause
// copyright-holders:Sebastien Monassa
/*************************************************************************

    Atari Video Pinball hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define VIDEOPIN_OCTAVE_DATA    NODE_01
#define VIDEOPIN_NOTE_DATA      NODE_02
#define VIDEOPIN_BELL_EN        NODE_03
#define VIDEOPIN_BONG_EN        NODE_04
#define VIDEOPIN_ATTRACT_EN     NODE_05
#define VIDEOPIN_VOL_DATA       NODE_06


class videopin_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT
	};

	videopin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_video_ram;

	attotime m_time_pushed;
	attotime m_time_released;
	uint8_t m_prev;
	uint8_t m_mask;
	int m_ball_x;
	int m_ball_y;
	tilemap_t* m_bg_tilemap;
	emu_timer *m_interrupt_timer;

	uint8_t misc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ball_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void note_dvsr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	tilemap_memory_index get_memory_offset(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void interrupt_callback(void *ptr, int32_t param);
	void update_plunger();
	double calc_plunger_pos();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in audio/videopin.c -----------*/
DISCRETE_SOUND_EXTERN( videopin );
