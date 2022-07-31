// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/
#ifndef MAME_INCLUDES_SUPRRIDR_H
#define MAME_INCLUDES_SUPRRIDR_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class suprridr_state : public driver_device
{
public:
	suprridr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram")
	{ }

	void suprridr(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(control_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_nmi_enable = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap_noscroll = nullptr;
	uint8_t m_flipx = 0;
	uint8_t m_flipy = 0;

	void nmi_enable_w(uint8_t data);
	void coin_lock_w(uint8_t data);
	void flipx_w(uint8_t data);
	void flipy_w(uint8_t data);
	void fgdisable_w(uint8_t data);
	void fgscrolly_w(uint8_t data);
	void bgscrolly_w(uint8_t data);
	void bgram_w(offs_t offset, uint8_t data);
	void fgram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	INTERRUPT_GEN_MEMBER(main_nmi_gen);

	void suprridr_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int is_screen_flipped();

	void main_map(address_map &map);
	void main_portmap(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};

#endif // MAME_INCLUDES_SUPRRIDR_H
