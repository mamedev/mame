// license:BSD-3-Clause
// copyright-holders:Curt Coder
// thanks-to:Kenneth Lin (original driver author)
#ifndef MAME_INCLUDES_JACKAL_H
#define MAME_INCLUDES_JACKAL_H

#pragma once

#include "emupal.h"
#include "tilemap.h"


class jackal_state : public driver_device
{
public:
	jackal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoctrl(*this, "videoctrl"),
		m_videoram(*this, "videoram%u", 0U),
		m_scrollram(*this, "scrollram%u", 0U, 0x40U, ENDIANNESS_BIG),
		m_mainbank(*this, "mainbank"),
		m_videoview(*this, "videoview"),
		m_spritebank(*this, "spritebank"),
		m_spriteram(*this, "spriteram%u", 0U, 0x1000U, ENDIANNESS_BIG),
		m_scrollbank(*this, "scrollbank"),
		m_dials(*this, "DIAL%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_slavecpu(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void jackal(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoctrl;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	memory_share_array_creator<uint8_t, 2> m_scrollram;
	required_memory_bank m_mainbank;
	memory_view m_videoview;
	required_memory_bank m_spritebank;
	memory_share_array_creator<uint8_t, 2> m_spriteram;
	required_memory_bank m_scrollbank;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	// misc
	uint8_t m_irq_enable = 0;
	optional_ioport_array<2> m_dials;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slavecpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t rotary_r(offs_t offset);
	void flipscreen_w(uint8_t data);
	void rambank_w(uint8_t data);
	template <uint8_t Which> void voram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_region(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *sram, int length, int bank);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
	void slave_map(address_map &map);
};

#endif // MAME_INCLUDES_JACKAL_H
