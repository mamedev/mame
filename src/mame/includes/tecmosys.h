// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

    tecmosys protection simulation

***************************************************************************/
#ifndef MAME_INCLUDES_TECMOSYS_H
#define MAME_INCLUDES_TECMOSYS_H

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
	virtual void machine_start() override;
	virtual void video_start() override;

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

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_tilemap_paletteram16;
	required_shared_ptr_array<uint16_t, 4> m_vram;
	required_shared_ptr_array<uint16_t, 3> m_lineram;
	required_shared_ptr_array<uint16_t, 4> m_scroll;
	required_shared_ptr<uint16_t> m_880000regs;

	required_region_ptr<uint8_t> m_sprite_region;

	required_memory_bank m_audiobank;
	required_memory_bank_array<2> m_okibank;

	int m_spritelist;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tmp_tilemap_composebitmap;
	bitmap_ind16 m_tmp_tilemap_renderbitmap;
	tilemap_t *m_tilemap[4];
	uint8_t m_device_read_ptr;
	uint8_t m_device_status;
	const struct prot_data* m_device_data;
	uint8_t m_device_value;

	DECLARE_READ8_MEMBER(sound_command_pending_r);
	DECLARE_WRITE8_MEMBER(sound_nmi_disable_w);
	DECLARE_WRITE16_MEMBER(unk880000_w);
	DECLARE_READ16_MEMBER(unk880000_r);
	DECLARE_WRITE8_MEMBER(z80_bank_w);
	DECLARE_WRITE8_MEMBER(oki_bank_w);
	DECLARE_READ16_MEMBER(prot_status_r);
	DECLARE_WRITE16_MEMBER(prot_status_w);
	DECLARE_READ16_MEMBER(prot_data_r);
	DECLARE_WRITE16_MEMBER(prot_data_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE16_MEMBER(tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(lineram_w);
	DECLARE_READ16_MEMBER(eeprom_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prot_init(int which);
	void prot_reset();
	inline void set_color_555(pen_t color, int rshift, int gshift, int bshift, uint16_t data);
	void render_sprites_to_bitmap(bitmap_rgb32 &bitmap, uint16_t extrax, uint16_t extray);
	void tilemap_copy_to_compose(uint16_t pri, const rectangle &cliprect);
	void do_final_mix(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void descramble();

	void io_map(address_map &map);
	void main_map(address_map &map);
	void oki_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_TECMOSYS_H
