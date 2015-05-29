// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#include "audio/warpwarp.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_warpwarp_sound(*this, "warpwarp_custom"),
		m_geebee_sound(*this, "geebee_custom"),
		m_geebee_videoram(*this, "geebee_videoram"),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<warpwarp_sound_device> m_warpwarp_sound;
	optional_device<geebee_sound_device> m_geebee_sound;
	optional_shared_ptr<UINT8> m_geebee_videoram;
	optional_shared_ptr<UINT8> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	int m_geebee_bgw;
	int m_ball_on;
	int m_ball_h;
	int m_ball_v;
	int m_ball_pen;
	int m_ball_sizex;
	int m_ball_sizey;
	int m_handle_joystick;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(geebee_in_r);
	DECLARE_WRITE8_MEMBER(geebee_out6_w);
	DECLARE_WRITE8_MEMBER(geebee_out7_w);
	DECLARE_READ8_MEMBER(warpwarp_sw_r);
	DECLARE_WRITE8_MEMBER(warpwarp_out0_w);
	DECLARE_WRITE8_MEMBER(warpwarp_out3_w);
	DECLARE_WRITE8_MEMBER(geebee_videoram_w);
	DECLARE_WRITE8_MEMBER(warpwarp_videoram_w);
	DECLARE_READ8_MEMBER(warpwarp_dsw1_r);
	DECLARE_READ8_MEMBER(warpwarp_vol_r);
	DECLARE_DRIVER_INIT(navarone);
	DECLARE_DRIVER_INIT(geebee);
	DECLARE_DRIVER_INIT(kaitein);
	DECLARE_DRIVER_INIT(warpwarp);
	DECLARE_DRIVER_INIT(sos);
	DECLARE_DRIVER_INIT(kaitei);
	DECLARE_DRIVER_INIT(bombbee);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(geebee_get_tile_info);
	TILE_GET_INFO_MEMBER(navarone_get_tile_info);
	TILE_GET_INFO_MEMBER(warpwarp_get_tile_info);
	DECLARE_VIDEO_START(geebee);
	DECLARE_PALETTE_INIT(geebee);
	DECLARE_VIDEO_START(warpwarp);
	DECLARE_PALETTE_INIT(warpwarp);
	DECLARE_VIDEO_START(navarone);
	DECLARE_PALETTE_INIT(navarone);
	UINT32 screen_update_geebee(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	inline void geebee_plot(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, pen_t pen);
	void draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen);
};
