// license:BSD-3-Clause
// copyright-holders:Curt Coder
// thanks-to:Kenneth Lin (original driver author)
#ifndef MAME_INCLUDES_JACKAL_H
#define MAME_INCLUDES_JACKAL_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

#define MASTER_CLOCK         XTAL(18'432'000)
#define SOUND_CLOCK          XTAL(3'579'545)



class jackal_state : public driver_device
{
public:
	jackal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoctrl(*this, "videoctrl"),
		m_dials(*this, "DIAL%u", 0U),
		m_mastercpu(*this, "master"),
		m_slavecpu(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void jackal(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoctrl;
	uint8_t *  m_scrollram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	int      m_irq_enable;
	uint8_t    *m_rambank;
	uint8_t    *m_spritebank;
	optional_ioport_array<2> m_dials;

	/* devices */
	required_device<cpu_device> m_mastercpu;
	required_device<cpu_device> m_slavecpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(jackalr_rotary_r);
	DECLARE_WRITE8_MEMBER(jackal_flipscreen_w);
	DECLARE_READ8_MEMBER(jackal_zram_r);
	DECLARE_READ8_MEMBER(jackal_voram_r);
	DECLARE_READ8_MEMBER(jackal_spriteram_r);
	DECLARE_WRITE8_MEMBER(jackal_rambank_w);
	DECLARE_WRITE8_MEMBER(jackal_zram_w);
	DECLARE_WRITE8_MEMBER(jackal_voram_w);
	DECLARE_WRITE8_MEMBER(jackal_spriteram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void jackal_palette(palette_device &palette) const;
	uint32_t screen_update_jackal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void jackal_mark_tile_dirty( int offset );
	void draw_background( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites_region( bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *sram, int length, int bank );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void master_map(address_map &map);
	void slave_map(address_map &map);
};

#endif // MAME_INCLUDES_JACKAL_H
