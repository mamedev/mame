// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_SBASKETB_H
#define MAME_INCLUDES_SBASKETB_H

#pragma once

#include "audio/trackfld.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class sbasketb_state : public driver_device
{
public:
	sbasketb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_palettebank(*this, "palettebank"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundbrd(*this, "trackfld_audio"),
		m_dac(*this, "dac"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void sbasketb(machine_config &config);
	void sbasketbu(machine_config &config);

	void init_sbasketb();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_scroll;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<trackfld_audio_device> m_soundbrd;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<sn76489_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	bool       m_spriteram_select;

	bool       m_irq_mask;
	void sbasketb_sh_irqtrigger_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	void sbasketb_videoram_w(offs_t offset, uint8_t data);
	void sbasketb_colorram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(spriteram_select_w);

	uint8_t m_SN76496_latch;
	void  konami_SN76496_latch_w(uint8_t data) { m_SN76496_latch = data; }
	void  konami_SN76496_w(uint8_t data) { m_sn->write(m_SN76496_latch); }
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	void sbasketb_palette(palette_device &palette) const;
	uint32_t screen_update_sbasketb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void sbasketb_map(address_map &map);
	void sbasketb_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SBASKETB_H
