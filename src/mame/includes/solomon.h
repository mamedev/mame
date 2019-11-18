// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#ifndef MAME_INCLUDES_SOLOMON_H
#define MAME_INCLUDES_SOLOMON_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class solomon_state : public driver_device
{
public:
	solomon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void solomon(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	uint8_t m_nmi_mask;
	DECLARE_WRITE8_MEMBER(solomon_sh_command_w);
	DECLARE_READ8_MEMBER(solomon_0xe603_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(solomon_videoram_w);
	DECLARE_WRITE8_MEMBER(solomon_colorram_w);
	DECLARE_WRITE8_MEMBER(solomon_videoram2_w);
	DECLARE_WRITE8_MEMBER(solomon_colorram2_w);
	DECLARE_WRITE8_MEMBER(solomon_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update_solomon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};

#endif // MAME_INCLUDES_SOLOMON_H
