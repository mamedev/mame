// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "emupal.h"
#include "screen.h"

class m52_state : public driver_device
{
public:
	m52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	void m52(machine_config &config);
	void alpha1v(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	uint8_t                m_bg1xpos;
	uint8_t                m_bg1ypos;
	uint8_t                m_bg2xpos;
	uint8_t                m_bg2ypos;
	uint8_t                m_bgcontrol;
	DECLARE_WRITE8_MEMBER(m52_scroll_w);
	DECLARE_WRITE8_MEMBER(m52_videoram_w);
	DECLARE_WRITE8_MEMBER(m52_colorram_w);
	DECLARE_READ8_MEMBER(m52_protection_r);
	DECLARE_WRITE8_MEMBER(m52_bg1ypos_w);
	DECLARE_WRITE8_MEMBER(m52_bg1xpos_w);
	DECLARE_WRITE8_MEMBER(m52_bg2xpos_w);
	DECLARE_WRITE8_MEMBER(m52_bg2ypos_w);
	DECLARE_WRITE8_MEMBER(m52_bgcontrol_w);
	DECLARE_WRITE8_MEMBER(m52_flipscreen_w);
	DECLARE_WRITE8_MEMBER(alpha1v_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(m52);
	uint32_t screen_update_m52(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int initoffs);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	void alpha1v_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
};
