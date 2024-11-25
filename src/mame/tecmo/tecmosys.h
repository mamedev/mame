// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

    tecmosys protection simulation

***************************************************************************/
#ifndef MAME_TECMO_TECMOSYS_H
#define MAME_TECMO_TECMOSYS_H

#pragma once

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class tecmosys_state : public driver_device
{
public:
	tecmosys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_eeprom(*this, "eeprom")
		, m_watchdog(*this, "watchdog")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundnmi(*this, "soundnmi")
		, m_spriteram(*this, "spriteram")
		, m_tilemap_paletteram16(*this, "tmap_palette")
		, m_vram(*this, "vram_%u", 0)
		, m_lineram(*this, "bg%u_lineram", 0)
		, m_scroll(*this, "scroll_%u", 0)
		, m_880000regs(*this, "880000regs")
		, m_sprite_region(*this, "sprites")
		, m_audiobank(*this, "audiobank")
		, m_okibank(*this, "okibank%u", 1)
	{
	}

	void tecmosys(machine_config &config);

	void init_tkdensha();
	void init_deroon();
	void init_tkdensho();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<input_merger_device> m_soundnmi;

	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_tilemap_paletteram16;
	required_shared_ptr_array<u16, 4> m_vram;
	required_shared_ptr_array<u16, 3> m_lineram;
	required_shared_ptr_array<u16, 4> m_scroll;
	required_shared_ptr<u16> m_880000regs;

	required_region_ptr<u8> m_sprite_region;
	std::unique_ptr<u8[]>   m_sprite_gfx;
	offs_t                  m_sprite_gfx_mask = 0;

	required_memory_bank m_audiobank;
	required_memory_bank_array<2> m_okibank;

	int m_spritelist = 0;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tmp_tilemap_composebitmap;
	bitmap_ind16 m_tmp_tilemap_renderbitmap;
	tilemap_t *m_tilemap[4]{};
	u8 m_device_read_ptr = 0;
	u8 m_device_status = 0;
	const struct prot_data* m_device_data = nullptr;
	u8 m_device_value = 0;

	u8 sound_command_pending_r();
	void sound_nmi_disable_w(u8 data);
	void unk880000_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 unk880000_r(offs_t offset);
	void z80_bank_w(u8 data);
	void oki_bank_w(u8 data);
	u16 prot_status_r(offs_t offset, u16 mem_mask = ~0);
	void prot_status_w(u16 data);
	u16 prot_data_r();
	void prot_data_w(u16 data);
	template<int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<int Layer> void lineram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 eeprom_r();
	void eeprom_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prot_init(int which);
	void prot_reset();
	inline void set_color_555(pen_t color, int rshift, int gshift, int bshift, u16 data);
	void render_sprites_to_bitmap(const rectangle &cliprect, u16 extrax, u16 extray);
	void tilemap_copy_to_compose(u16 pri, const rectangle &cliprect);
	void do_final_mix(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void descramble();

	void io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TECMO_TECMOSYS_H
