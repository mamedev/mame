// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Jailbreak

***************************************************************************/
#ifndef MAME_INCLUDES_JAILBREK_H
#define MAME_INCLUDES_JAILBREK_H

#pragma once

#include "sound/vlm5030.h"
#include "emupal.h"
#include "tilemap.h"

#define MASTER_CLOCK        XTAL(18'432'000)
#define VOICE_CLOCK         XTAL(3'579'545)

class jailbrek_state : public driver_device
{
public:
	jailbrek_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_dir(*this, "scroll_dir"),
		m_maincpu(*this, "maincpu"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void jailbrek(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_scroll_dir;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t      *m_bg_tilemap = nullptr;

	/* misc */
	uint8_t        m_irq_enable = 0U;
	uint8_t        m_nmi_enable = 0U;
	void ctrl_w(uint8_t data);
	void coin_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t speech_r();
	void speech_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void jailbrek_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(interrupt_nmi);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void jailbrek_map(address_map &map);
	void vlm_map(address_map &map);
};

#endif // MAME_INCLUDES_JAILBREK_H
