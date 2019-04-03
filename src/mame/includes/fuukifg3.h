// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
#ifndef MAME_INCLUDES_FUUKIFG3_H
#define MAME_INCLUDES_FUUKIFG3_H

#pragma once

#include "video/fuukifg.h"
#include "emupal.h"
#include "screen.h"

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK       (XTAL(40'000'000) / 2)        /* clock for 68020 */
#define SOUND_CPU_CLOCK     (XTAL(12'000'000) / 2)        /* clock for Z80 sound CPU */

/* NOTE: YMF278B_STD_CLOCK is defined in /src/emu/sound/ymf278b.h */


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
		, m_vram(*this, "vram.%u", 0)
		, m_vregs(*this, "vregs")
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
	enum
	{
		TIMER_LEVEL_1_INTERRUPT,
		TIMER_VBLANK_INTERRUPT,
		TIMER_RASTER_INTERRUPT
	};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<fuukivid_device> m_fuukivid;

	/* memory pointers */
	required_shared_ptr_array<uint32_t,4> m_vram;
	required_shared_ptr<uint32_t> m_vregs;
	required_shared_ptr<uint32_t> m_priority;
	required_shared_ptr<uint32_t> m_tilebank;
	required_shared_ptr<uint8_t> m_shared_ram;
	//uint32_t *    m_buf_spriteram;
	//uint32_t *    m_buf_spriteram2;

	required_memory_bank m_soundbank;

	required_ioport m_system;
	required_ioport m_inputs;
	required_ioport m_dsw1;
	required_ioport m_dsw2;

	/* video-related */
	tilemap_t     *m_tilemap[4];
	uint32_t      m_spr_buffered_tilebank[2];

	/* misc */
	emu_timer   *m_level_1_interrupt_timer;
	emu_timer   *m_vblank_interrupt_timer;
	emu_timer   *m_raster_interrupt_timer;

	DECLARE_READ8_MEMBER(snd_020_r);
	DECLARE_WRITE8_MEMBER(snd_020_w);
	DECLARE_WRITE32_MEMBER(vregs_w);
	DECLARE_WRITE8_MEMBER(sound_bw_w);
	DECLARE_WRITE8_MEMBER(snd_ymf278b_w);
	template<int Layer> DECLARE_WRITE32_MEMBER(vram_w);

	template<int Layer, int ColShift> TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void draw_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int flag, int pri );

	void fuuki32_map(address_map &map);
	void fuuki32_sound_io_map(address_map &map);
	void fuuki32_sound_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_FUUKIFG3_H
