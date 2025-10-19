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

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

struct M92_pf_layer_info
{
	tilemap_t *tmap = nullptr;
	tilemap_t *wide_tmap = nullptr;
	u16 vram_base = 0;
	u16 control[4]{};
};

class m92_state : public driver_device
{
public:
	m92_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram", 0x2000, ENDIANNESS_LITTLE),
		m_spriteram(*this, "spriteram", 0x1000, ENDIANNESS_LITTLE),
		m_spriteram_buffer(*this, "spriteram_buffer", 0x800, ENDIANNESS_LITTLE),
		m_vram_data(*this, "vram_data"),
		m_dmacontrol(*this, "dmacontrol"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
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
	void hook(machine_config &config);
	void psoldier(machine_config &config);
	void rtypeleo(machine_config &config);
	void gunforc2(machine_config &config);
	void geostorma(machine_config &config);
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

	int dma_busy_r() { return m_dma_busy; }

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD { m_dma_busy = 1; }

	memory_share_creator<u16> m_paletteram;
	memory_share_creator<u16> m_spriteram;
	memory_share_creator<u16> m_spriteram_buffer;
	required_shared_ptr<u16> m_vram_data;
	required_shared_ptr<u16> m_dmacontrol;
	required_device<cpu_device> m_maincpu;
	optional_device<v35_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_upd71059c;

	optional_memory_bank m_mainbank;
	required_ioport m_dsw;

	emu_timer *m_dma_timer = nullptr;
	s32 m_raster_irq_position = -1;
	u16 m_videocontrol = 0;
	u8 m_dma_busy = 0;
	M92_pf_layer_info m_pf_layer[3];
	u16 m_pf_master_control[4]{};
	std::vector<u16> m_paletteram_buffer;

	void coincounter_w(u8 data);
	void bankswitch_w(u8 data);
	void sound_reset_w(u16 data);
	void sprite_dma(int amount);
	void dmacontrol_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void videocontrol_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<int Layer> void pf_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void master_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	u32 screen_update_m92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m92_update_scroll_positions();
	void m92_draw_tiles(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void lethalth_map(address_map &map) ATTR_COLD;
	void m92_map(address_map &map) ATTR_COLD;
	void m92_banked_map(address_map &map) ATTR_COLD;
	void m92_banked_portmap(address_map &map) ATTR_COLD;
	void m92_base_map(address_map &map) ATTR_COLD;
	void m92_portmap(address_map &map) ATTR_COLD;
	void majtitl2_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(dma_done);
};


// Peter Pan (bootleg of Hook, OKI sound, different sprite hardware)
class ppan_state : public m92_state
{
public:
	ppan_state(const machine_config &mconfig, device_type type, const char *tag) :
		m92_state(mconfig, type, tag),
		m_oki(*this, "oki")
	{ }

	void ppan(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	required_device<okim6295_device> m_oki;

	void ppan_map(address_map &map) ATTR_COLD;
	void ppan_portmap(address_map &map) ATTR_COLD;
};


// Ninja Baseball Bat Man II (bootleg, very different hardware)
class nbb2b_state : public m92_state
{
public:
	nbb2b_state(const machine_config &mconfig, device_type type, const char *tag) :
		m92_state(mconfig, type, tag)
	{ }

	void nbbatman2bl(machine_config &config);

private:
	void nbbatman2bl_map(address_map &map) ATTR_COLD;
};

#endif // MAME_IREM_M92_H
