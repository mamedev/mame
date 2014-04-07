#include "includes/kaneko16.h"

class galpanic_state : public kaneko16_state
{
public:
	galpanic_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
			m_bgvideoram(*this, "bgvideoram"),
			m_fgvideoram(*this, "fgvideoram"),
			m_spriteram(*this, "spriteram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_fgvideoram;
	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_sprites_bitmap;
	optional_shared_ptr<UINT16> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(galpanic_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(galpanica_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(galpanica_misc_w);
	DECLARE_WRITE16_MEMBER(galpanic_coin_w);
	DECLARE_WRITE16_MEMBER(galpanic_bgvideoram_mirror_w);
	DECLARE_READ16_MEMBER(comad_timer_r);
	DECLARE_READ16_MEMBER(zipzap_random_read);
	DECLARE_READ8_MEMBER(comad_okim6295_r);
	DECLARE_VIDEO_START(galpanic);
	DECLARE_PALETTE_INIT(galpanic);
	UINT32 screen_update_galpanic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_comad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_galpanic(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(galpanic_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(galhustl_scanline);
	void comad_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_fgbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	/*----------- defined in video/galpanic.c -----------*/
	DECLARE_WRITE16_MEMBER( galpanic_bgvideoram_w );
	DECLARE_WRITE16_MEMBER( galpanic_paletteram_w );
};
