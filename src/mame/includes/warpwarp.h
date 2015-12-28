// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#include "audio/warpwarp.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
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
		m_ports(*this, portnames)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<warpwarp_sound_device> m_warpwarp_sound;
	optional_device<geebee_sound_device> m_geebee_sound;
	optional_shared_ptr<UINT8> m_geebee_videoram;
	optional_shared_ptr<UINT8> m_videoram;
	optional_ioport m_in0;
	optional_ioport m_in1;
	optional_ioport m_in2;
	optional_ioport m_dsw1;
	optional_ioport m_volin1;
	optional_ioport m_volin2;
	DECLARE_IOPORT_ARRAY(portnames);
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
	DECLARE_READ8_MEMBER(warpwarp_sw_r);
	DECLARE_WRITE8_MEMBER(warpwarp_out0_w);
	DECLARE_WRITE8_MEMBER(warpwarp_out3_w);
	DECLARE_WRITE8_MEMBER(warpwarp_videoram_w);
	DECLARE_READ8_MEMBER(warpwarp_dsw1_r);
	DECLARE_READ8_MEMBER(warpwarp_vol_r);

	//geebee and navarone
	DECLARE_READ8_MEMBER(geebee_in_r);
	DECLARE_WRITE8_MEMBER(geebee_out6_w);
	DECLARE_WRITE8_MEMBER(geebee_out7_w);
	DECLARE_WRITE8_MEMBER(geebee_videoram_w);

	virtual void machine_start() override;
	DECLARE_DRIVER_INIT(navarone);
	DECLARE_DRIVER_INIT(geebee);
	DECLARE_DRIVER_INIT(kaitein);
	DECLARE_DRIVER_INIT(warpwarp);
	DECLARE_DRIVER_INIT(sos);
	DECLARE_DRIVER_INIT(kaitei);
	DECLARE_DRIVER_INIT(bombbee);
	DECLARE_VIDEO_START(geebee);
	DECLARE_PALETTE_INIT(geebee);
	DECLARE_VIDEO_START(warpwarp);
	DECLARE_PALETTE_INIT(warpwarp);
	DECLARE_VIDEO_START(navarone);
	DECLARE_PALETTE_INIT(navarone);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(geebee_get_tile_info);
	TILE_GET_INFO_MEMBER(navarone_get_tile_info);
	TILE_GET_INFO_MEMBER(warpwarp_get_tile_info);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void plot(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, pen_t pen);
	void draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen);

	INTERRUPT_GEN_MEMBER(vblank_irq);
};
