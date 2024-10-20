// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Irem M92 hardware

*************************************************************************/
#ifndef MAME_IREM_M92_H
#define MAME_IREM_M92_H

#pragma once

#include "cpu/nec/v25.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

struct M92_pf_layer_info
{
	tilemap_t *     tmap = nullptr;
	tilemap_t *     wide_tmap = nullptr;
	uint16_t          vram_base = 0;
	uint16_t          control[4]{};
};

class m92_state : public driver_device
{
public:
	m92_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_vram_data(*this, "vram_data"),
		m_spritecontrol(*this, "spritecontrol"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_upd71059c(*this, "upd71059c"),
		m_mainbank(*this, "mainbank"),
		m_dsw(*this, "DSW")
	{ }

	void m92(machine_config &config);
	void m92_banked(machine_config &config);
	void inthunt(machine_config &config);
	void lethalth(machine_config &config);
	void ppan(machine_config &config);
	void hook(machine_config &config);
	void psoldier(machine_config &config);
	void rtypeleo(machine_config &config);
	void gunforc2(machine_config &config);
	void geostorma(machine_config &config);
	void nbbatman2bl(machine_config &config);
	void bmaster(machine_config &config);
	void nbbatman(machine_config &config);
	void uccops(machine_config &config);
	void dsoccr94j(machine_config &config);
	void gunforce(machine_config &config);
	void majtitl2(machine_config &config);
	void majtitl2a(machine_config &config);
	void mysticri(machine_config &config);
	void leaguemna(machine_config &config);

	void init_bank();

	int sprite_busy_r();

private:
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_vram_data;
	required_shared_ptr<uint16_t> m_spritecontrol;
	required_device<cpu_device> m_maincpu;
	optional_device<v35_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_upd71059c;

	optional_memory_bank m_mainbank;
	required_ioport m_dsw;

	emu_timer *m_spritebuffer_timer = nullptr;
	uint32_t m_raster_irq_position = 0;
	uint16_t m_videocontrol = 0;
	uint8_t m_sprite_buffer_busy = 0;
	M92_pf_layer_info m_pf_layer[3];
	uint16_t m_pf_master_control[4]{};
	int32_t m_sprite_list = 0;
	uint8_t m_palette_bank = 0;
	std::vector<uint16_t> m_paletteram;

	void coincounter_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	void sound_reset_w(uint16_t data);
	void spritecontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void videocontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void pf_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void master_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void oki_bank_w(uint16_t data);
	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	DECLARE_MACHINE_RESET(m92);
	DECLARE_VIDEO_START(m92);
	DECLARE_VIDEO_START(ppan);
	uint32_t screen_update_m92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ppan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ppan_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m92_update_scroll_positions();
	void m92_draw_tiles(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);

	void lethalth_map(address_map &map) ATTR_COLD;
	void m92_map(address_map &map) ATTR_COLD;
	void m92_banked_map(address_map &map) ATTR_COLD;
	void m92_banked_portmap(address_map &map) ATTR_COLD;
	void m92_base_map(address_map &map) ATTR_COLD;
	void m92_portmap(address_map &map) ATTR_COLD;
	void majtitl2_map(address_map &map) ATTR_COLD;
	void ppan_portmap(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(spritebuffer_done);
};

#endif // MAME_IREM_M92_H
