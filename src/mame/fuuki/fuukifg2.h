// license:BSD-3-Clause
// copyright-holders:Luca Elia,Paul Priest
#ifndef MAME_FUUKI_FUUKIFG2_H
#define MAME_FUUKI_FUUKIFG2_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "fuukifg.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class fuuki16_state : public driver_device
{
public:
	fuuki16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_fuukivid(*this, "fuukivid")
		, m_soundlatch(*this, "soundlatch")
		, m_spriteram(*this, "spriteram")
		, m_vram(*this, "vram.%u", 0)
		, m_vregs(*this, "vregs")
		, m_unknown(*this, "unknown")
		, m_priority(*this, "priority")
		, m_soundbank(*this, "soundbank")
	{ }

	void fuuki16(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(level1_interrupt);
	TIMER_CALLBACK_MEMBER(vblank_interrupt);
	TIMER_CALLBACK_MEMBER(raster_interrupt);

	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sound_command_w(u8 data);
	void sound_rombank_w(u8 data);
	template<int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<int Layer> void vram_buffered_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void oki_banking_w(u8 data);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	void fuuki16_colpri_cb(u32 &colour, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 i, int flag, u8 pri, u8 primask = 0xff);

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<fuukivid_device> m_fuukivid;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr_array<u16, 4> m_vram;
	required_shared_ptr<u16> m_vregs;
	required_shared_ptr<u16> m_unknown;
	required_shared_ptr<u16> m_priority;

	required_memory_bank m_soundbank;

	/* video-related */
	tilemap_t     *m_tilemap[3]{};

	/* misc */
	emu_timer   *m_level_1_interrupt_timer = nullptr;
	emu_timer   *m_vblank_interrupt_timer = nullptr;
	emu_timer   *m_raster_interrupt_timer = nullptr;
};

#endif // MAME_FUUKI_FUUKIFG2_H
