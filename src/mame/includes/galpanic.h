// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "includes/kaneko16.h"

class galpanic_state : public driver_device
{
public:
	galpanic_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_bgvideoram(*this, "bgvideoram"),
			m_fgvideoram(*this, "fgvideoram"),
			m_spriteram(*this, "spriteram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_pandora(*this, "pandora")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_fgvideoram;
	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_sprites_bitmap;
	optional_shared_ptr<UINT16> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<kaneko_pandora_device> m_pandora;

	DECLARE_WRITE16_MEMBER(galpanic_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(galpanic_coin_w);

	DECLARE_VIDEO_START(galpanic);
	DECLARE_PALETTE_INIT(galpanic);
	UINT32 screen_update_galpanic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_galpanic(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(galpanic_scanline);
	void draw_fgbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	/*----------- defined in video/galpanic.c -----------*/
	DECLARE_WRITE16_MEMBER( galpanic_bgvideoram_w );
};
