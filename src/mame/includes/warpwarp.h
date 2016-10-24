// license:BSD-3-Clause
// copyright-holders:Chris Hardy

#include "machine/watchdog.h"
#include "audio/geebee.h"
#include "audio/warpwarp.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_warpwarp_sound(*this, "warpwarp_custom"),
		m_geebee_sound(*this, "geebee_custom"),
		m_geebee_videoram(*this, "geebee_videoram"),
		m_videoram(*this, "videoram"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_dsw1(*this, "DSW1"),
		m_volin1(*this, "VOLIN1"),
		m_volin2(*this, "VOLIN2"),
		m_ports(*this, { { "SW0", "SW1", "DSW2", "PLACEHOLDER" } }) // "IN1" & "IN2" are read separately when offset==3
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<warpwarp_sound_device> m_warpwarp_sound;
	optional_device<geebee_sound_device> m_geebee_sound;
	optional_shared_ptr<uint8_t> m_geebee_videoram;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_ioport m_in0;
	optional_ioport m_in1;
	optional_ioport m_in2;
	optional_ioport m_dsw1;
	optional_ioport m_volin1;
	optional_ioport m_volin2;
	optional_ioport_array<4> m_ports;

	int m_geebee_bgw;
	int m_ball_on;
	int m_ball_h;
	int m_ball_v;
	int m_ball_pen;
	int m_ball_sizex;
	int m_ball_sizey;
	int m_handle_joystick;
	tilemap_t *m_bg_tilemap;

	// warpwarp and bombbee
	uint8_t warpwarp_sw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void warpwarp_out0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void warpwarp_out3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void warpwarp_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t warpwarp_dsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t warpwarp_vol_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	//geebee and navarone
	uint8_t geebee_in_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void geebee_out6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void geebee_out7_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void geebee_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	void init_navarone();
	void init_geebee();
	void init_kaitein();
	void init_warpwarp();
	void init_sos();
	void init_kaitei();
	void init_bombbee();
	void video_start_geebee();
	void palette_init_geebee(palette_device &palette);
	void video_start_warpwarp();
	void palette_init_warpwarp(palette_device &palette);
	void video_start_navarone();
	void palette_init_navarone(palette_device &palette);

	tilemap_memory_index tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void geebee_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void navarone_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void warpwarp_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void plot(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, pen_t pen);
	void draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen);

	void vblank_irq(device_t &device);
};
