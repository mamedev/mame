// license:BSD-3-Clause
// copyright-holders:BUT
#ifndef MAME_NAMCO_TCEPTOR_H
#define MAME_NAMCO_TCEPTOR_H

#pragma once

#include "sound/namco.h"
#include "namco_c45road.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class tceptor_state : public driver_device
{
public:
	tceptor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu%u", 1U),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_tile_ram(*this, "tile_ram"),
		m_tile_attr(*this, "tile_attr"),
		m_bg_ram(*this, "bg_ram"),
		m_m68k_shared_ram(*this, "m68k_shared_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_c45_road(*this, "c45_road"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_inp(*this, "IN%u", 0U),
		m_dsw(*this, "DSW%u", 1U),
		m_shutter(*this, "shutter")
	{ }

	void tceptor(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<cpu_device, 2> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;

	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_attr;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_m68k_shared_ram;
	required_shared_ptr<uint16_t> m_sprite_ram;

	required_device<namco_c45_road_device> m_c45_road;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_inp;
	required_ioport_array<2> m_dsw;
	output_finder<> m_shutter;

	uint8_t m_m6809_irq_enable = 0;
	uint8_t m_m68k_irq_enable = 0;
	uint8_t m_mcu_irq_enable = 0;
	int m_sprite16 = 0;
	int m_sprite32 = 0;
	int m_bg = 0;
	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};
	int32_t m_bg_scroll_x[2]{};
	int32_t m_bg_scroll_y[2]{};
	bitmap_ind16 m_temp_bitmap;
	std::unique_ptr<uint16_t[]> m_sprite_ram_buffered;
	std::unique_ptr<uint8_t[]> m_decoded_16;
	std::unique_ptr<uint8_t[]> m_decoded_32;
	int m_is_mask_spr[1024/16]{};

	uint8_t m68k_shared_r(offs_t offset);
	void m68k_shared_w(offs_t offset, uint8_t data);
	void m6809_irq_enable_w(uint8_t data);
	void m6809_irq_disable_w(uint8_t data);
	void m68k_irq_enable_w(uint16_t data);
	void mcu_irq_enable_w(uint8_t data);
	void mcu_irq_disable_w(uint8_t data);
	uint8_t dsw0_r();
	uint8_t dsw1_r();
	uint8_t input0_r();
	uint8_t input1_r();
	void tceptor_tile_ram_w(offs_t offset, uint8_t data);
	void tceptor_tile_attr_w(offs_t offset, uint8_t data);
	void tceptor_bg_ram_w(offs_t offset, uint8_t data);
	void tceptor_bg_scroll_w(offs_t offset, uint8_t data);
	void tceptor2_shutter_w(uint8_t data);
	void tile_mark_dirty(int offset);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	void tceptor_palette(palette_device &palette);
	uint32_t screen_update_tceptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_tceptor(int state);
	INTERRUPT_GEN_MEMBER(m6809_vb_interrupt);
	INTERRUPT_GEN_MEMBER(m68k_vb_interrupt);
	INTERRUPT_GEN_MEMBER(mcu_vb_interrupt);
	inline int get_tile_addr(int tile_index);
	void decode_bg(const char * region);
	void decode_sprite(int gfx_index, const gfx_layout *layout, const void *data);
	void decode_sprite16(const char * region);
	void decode_sprite32(const char * region);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority);
	inline uint8_t fix_input0(uint8_t in1, uint8_t in2);
	inline uint8_t fix_input1(uint8_t in1, uint8_t in2);

	void m6502_a_map(address_map &map) ATTR_COLD;
	void m6502_b_map(address_map &map) ATTR_COLD;
	void m6809_map(address_map &map) ATTR_COLD;
	void m68k_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_TCEPTOR_H
