// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
#ifndef MAME_FUUKI_FUUKIFG3_H
#define MAME_FUUKI_FUUKIFG3_H

#pragma once

#include "fuukifg.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK       (XTAL(40'000'000) / 2)        /* clock for 68020 */
#define SOUND_CPU_CLOCK     (XTAL(12'000'000) / 2)        /* clock for Z80 sound CPU */


class fuuki32_state : public driver_device
{
public:
	fuuki32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_fuukivid(*this, "fuukivid")
		, m_spriteram(*this, "spriteram", 0x2000, ENDIANNESS_BIG)
		, m_vram(*this, "vram.%u", 0)
		, m_vregs(*this, "vregs", 0x20, ENDIANNESS_BIG)
		, m_priority(*this, "priority")
		, m_tilebank(*this, "tilebank")
		, m_shared_ram(*this, "shared_ram")
		, m_soundbank(*this, "soundbank")
		, m_system(*this, "SYSTEM")
		, m_inputs(*this, "INPUTS")
		, m_dsw1(*this, "DSW1")
		, m_dsw2(*this, "DSW2")
	{ }

	void fuuki32(machine_config &config);

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

	u8 snd_020_r(offs_t offset);
	void snd_020_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	void sprram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sprram_r(offs_t offset);
	u16 vregs_r(offs_t offset);
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sound_bw_w(u8 data);
	template<int Layer> void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Layer> void vram_buffered_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template<int Layer, int ColShift> TILE_GET_INFO_MEMBER(get_tile_info);

	void tile_cb(u32 &code);
	void colpri_cb(u32 &colour, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 i, int flag, u8 pri, u8 primask = 0xff);

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<fuukivid_device> m_fuukivid;

	/* memory pointers */
	memory_share_creator<u16> m_spriteram;
	required_shared_ptr_array<u32, 4> m_vram;
	memory_share_creator<u16> m_vregs;
	required_shared_ptr<u32> m_priority;
	required_shared_ptr<u32> m_tilebank;
	required_shared_ptr<u8> m_shared_ram;
	std::unique_ptr<u16[]> m_buf_spriteram[2];

	required_memory_bank m_soundbank;

	required_ioport m_system;
	required_ioport m_inputs;
	required_ioport m_dsw1;
	required_ioport m_dsw2;

	/* video-related */
	tilemap_t     *m_tilemap[3]{};
	u32      m_spr_buffered_tilebank[2]{};

	/* misc */
	emu_timer   *m_level_1_interrupt_timer = nullptr;
	emu_timer   *m_vblank_interrupt_timer = nullptr;
	emu_timer   *m_raster_interrupt_timer = nullptr;
};

#endif // MAME_FUUKI_FUUKIFG3_H
