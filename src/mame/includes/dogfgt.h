// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_DOGFGT_H
#define MAME_INCLUDES_DOGFGT_H

#pragma once

#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define PIXMAP_COLOR_BASE  (16 + 32)
#define BITMAPRAM_SIZE      0x6000


class dogfgt_state : public driver_device
{
public:
	dogfgt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sharedram(*this, "sharedram"),
		m_subcpu(*this, "sub") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ay(*this, "ay%u", 0U)
	{ }

	void dogfgt(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_sharedram;

	/* devices */
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<ay8910_device, 2> m_ay;

	/* video-related */
	bitmap_ind16 m_pixbitmap;
	tilemap_t   *m_bg_tilemap;
	std::unique_ptr<uint8_t[]>     m_bitmapram;
	int       m_bm_plane;
	int       m_pixcolor;
	int       m_scroll[4];
	int       m_lastflip;
	int       m_lastpixcolor;

	/* sound-related */
	int       m_soundlatch;
	int       m_last_snd_ctrl;

	DECLARE_WRITE8_MEMBER(subirqtrigger_w);
	DECLARE_WRITE8_MEMBER(sub_irqack_w);
	DECLARE_WRITE8_MEMBER(soundlatch_w);
	DECLARE_WRITE8_MEMBER(soundcontrol_w);
	DECLARE_WRITE8_MEMBER(plane_select_w);
	DECLARE_READ8_MEMBER(bitmapram_r);
	DECLARE_WRITE8_MEMBER(internal_bitmapram_w);
	DECLARE_WRITE8_MEMBER(bitmapram_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE8_MEMBER(_1800_w);


	TILE_GET_INFO_MEMBER(get_tile_info);
	void dogfgt_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void main_map(address_map &map);
	void sub_map(address_map &map);
};

#endif // MAME_INCLUDES_DOGFGT_H
