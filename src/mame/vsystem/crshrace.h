// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_CRSHRACE_H
#define MAME_INCLUDES_CRSHRACE_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/vsystem_spr.h"
#include "video/k053936.h"

#include "emupal.h"
#include "tilemap.h"


class crshrace_state : public driver_device
{
public:
	crshrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram.%u", 0U),
		m_z80bank(*this, "bank1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr(*this, "vsystem_spr"),
		m_k053936(*this, "k053936"),
		m_spriteram(*this, "spriteram.%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void crshrace(machine_config &config);

	void init_crshrace2();
	void init_crshrace();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_videoram;

	required_memory_bank m_z80bank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<vsystem_spr_device> m_spr;
	required_device<k053936_device> m_k053936;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	// video-related
	tilemap_t *m_tilemap[2]{};
	uint8_t m_roz_bank = 0U;
	uint8_t m_gfxctrl = 0U;
	uint8_t m_flipscreen = 0U;

	uint32_t tile_callback(uint32_t code);
	void sh_bankswitch_w(uint8_t data);
	template<uint8_t Which> void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_videoram[Which][offset]); m_tilemap[Which]->mark_tile_dirty(offset); }
	void roz_bank_w(offs_t offset, uint8_t data);
	void gfxctrl_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);

	[[maybe_unused]] void patch_code(uint16_t offset);
};

#endif // MAME_INCLUDES_CRSHRACE_H
