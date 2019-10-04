// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Irem M107 hardware

*************************************************************************/
#ifndef MAME_INCLUDES_M107_H
#define MAME_INCLUDES_M107_H

#pragma once

#include "cpu/nec/v25.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

struct pf_layer_info
{
	tilemap_t *     tmap;
	uint16_t        vram_base;
};

class m107_state : public driver_device
{
public:
	m107_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_upd71059c(*this, "upd71059c")
		, m_spriteram(*this, "spriteram")
		, m_vram_data(*this, "vram_data")
		, m_sprtable_rom(*this, "sprtable")
		, m_mainbank(*this, "mainbank")
	{
	}

	void airass(machine_config &config);
	void wpksoc(machine_config &config);
	void firebarr(machine_config &config);
	void dsoccr94(machine_config &config);

	void init_firebarr();
	void init_dsoccr94();
	void init_wpksoc();

private:
	required_device<cpu_device> m_maincpu;
	required_device<v35_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_upd71059c;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<uint16_t> m_vram_data;
	optional_region_ptr<uint8_t> m_sprtable_rom;

	optional_memory_bank m_mainbank;

	// driver init
	uint8_t m_spritesystem;

	uint8_t m_sprite_display;
	uint16_t m_raster_irq_position;
	pf_layer_info m_pf_layer[4];
	uint16_t m_control[0x10];

	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_reset_w);
	DECLARE_WRITE16_MEMBER(wpksoc_output_w);
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_WRITE16_MEMBER(spritebuffer_w);

	TILE_GET_INFO_MEMBER(get_pf_tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_scroll_positions();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int laynum, int category,int opaque);
	void screenrefresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dsoccr94_io_map(address_map &map);
	void dsoccr94_map(address_map &map);
	void firebarr_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
	void sound_map(address_map &map);
	void wpksoc_io_map(address_map &map);
	void wpksoc_map(address_map &map);
};

#endif // MAME_INCLUDES_M107_H
