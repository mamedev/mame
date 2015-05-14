// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
#include "sound/upd7759.h"

class snk68_state : public driver_device
{
public:
	snk68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_pow_fg_videoram(*this, "pow_fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	int m_invert_controls;
	int m_sound_status;

	required_shared_ptr<UINT16> m_pow_fg_videoram;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_paletteram;
	int m_sprite_flip_axis;
	tilemap_t *m_fg_tilemap;
	int m_flipscreen;
	UINT32 m_fg_tile_offset;
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_status_w);
	DECLARE_READ16_MEMBER(control_1_r);
	DECLARE_READ16_MEMBER(control_2_r);
	DECLARE_READ16_MEMBER(rotary_1_r);
	DECLARE_READ16_MEMBER(rotary_2_r);
	DECLARE_READ16_MEMBER(rotary_lsb_r);
	DECLARE_READ16_MEMBER(protcontrols_r);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(pow_spriteram_r);
	DECLARE_WRITE16_MEMBER(pow_spriteram_w);
	DECLARE_READ16_MEMBER(pow_fg_videoram_r);
	DECLARE_WRITE16_MEMBER(pow_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(searchar_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(pow_flipscreen16_w);
	DECLARE_WRITE16_MEMBER(searchar_flipscreen16_w);
	DECLARE_WRITE16_MEMBER(pow_paletteram16_word_w);
	DECLARE_WRITE8_MEMBER(D7759_write_port_0_w);
	DECLARE_WRITE8_MEMBER(D7759_upd_reset_w);
	DECLARE_DRIVER_INIT(searchar);
	TILE_GET_INFO_MEMBER(get_pow_tile_info);
	TILE_GET_INFO_MEMBER(get_searchar_tile_info);
	virtual void video_start();
	DECLARE_VIDEO_START(searchar);
	UINT32 screen_update_pow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_video_start();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
