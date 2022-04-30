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
	bitmap_ind16 m_pixbitmap{};
	tilemap_t   *m_bg_tilemap = nullptr;
	std::unique_ptr<uint8_t[]>     m_bitmapram{};
	int       m_bm_plane = 0;
	int       m_pixcolor = 0;
	int       m_scroll[4]{};
	int       m_lastflip = 0;
	int       m_lastpixcolor = 0;

	/* sound-related */
	int       m_soundlatch = 0;
	int       m_last_snd_ctrl = 0;

	void subirqtrigger_w(uint8_t data);
	void sub_irqack_w(uint8_t data);
	void soundlatch_w(uint8_t data);
	void soundcontrol_w(uint8_t data);
	void plane_select_w(uint8_t data);
	uint8_t bitmapram_r(offs_t offset);
	void internal_bitmapram_w(offs_t offset, uint8_t data);
	void bitmapram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	void _1800_w(uint8_t data);


	TILE_GET_INFO_MEMBER(get_tile_info);
	void dogfgt_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void main_map(address_map &map);
	void sub_map(address_map &map);
};

#endif // MAME_INCLUDES_DOGFGT_H
