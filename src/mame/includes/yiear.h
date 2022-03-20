// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
// thanks-to:Enrique Sanchez
#ifndef MAME_INCLUDES_YIEAR_H
#define MAME_INCLUDES_YIEAR_H

#pragma once

#include "audio/trackfld.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class yiear_state : public driver_device
{
public:
	yiear_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audio(*this, "trackfld_audio"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void yiear(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_videoram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<trackfld_audio_device> m_audio;
	required_device<sn76489a_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	uint8_t      m_nmi_enable;
	uint8_t      m_irq_enable;
	void videoram_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	uint8_t speech_r();
	void VLM5030_control_w(uint8_t data);

	uint8_t m_SN76496_latch;
	void konami_SN76496_latch_w(uint8_t data) { m_SN76496_latch = data; }
	void konami_SN76496_w(uint8_t data) { m_sn->write(m_SN76496_latch); }
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(nmi_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void main_map(address_map &map);
	void vlm_map(address_map &map);
};

#endif // MAME_INCLUDES_YIEAR_H
