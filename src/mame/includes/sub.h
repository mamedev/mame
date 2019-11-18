// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*************************************************************************************

Submarine (c) 1985 Sigma

*************************************************************************************/
#ifndef MAME_INCLUDES_SUB_H
#define MAME_INCLUDES_SUB_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define MASTER_CLOCK            XTAL(18'432'000)

class sub_state : public driver_device
{
public:
	sub_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_vram(*this, "vram"),
		m_attr(*this, "attr"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_scrolly(*this, "scrolly")
	{ }

	void sub(machine_config &config);

private:
	bool m_int_en;
	bool m_nmi_en;

	DECLARE_WRITE_LINE_MEMBER(int_mask_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);

	void sub_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_WRITE8_MEMBER(attr_w);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(main_irq);
	INTERRUPT_GEN_MEMBER(sound_irq);
	void subm_io(address_map &map);
	void subm_map(address_map &map);
	void subm_sound_io(address_map &map);
	void subm_sound_map(address_map &map);

	virtual void machine_start() override;
	virtual void video_start() override;

	tilemap_t *m_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_attr;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_scrolly;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif // MAME_INCLUDES_SUB_H
