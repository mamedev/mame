// license:BSD-3-Clause
// copyright-holders:BUT
#ifndef MAME_INCLUDES_TCEPTOR_H
#define MAME_INCLUDES_TCEPTOR_H

#pragma once

#include "cpu/m6502/m65c02.h"
#include "sound/namco.h"
#include "video/c45.h"
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
		m_shutter(*this, "shutter")
	{ }

	void tceptor(machine_config &config);

private:
	uint8_t m_m6809_irq_enable;
	uint8_t m_m68k_irq_enable;
	uint8_t m_mcu_irq_enable;
	required_device<cpu_device> m_maincpu;
	required_device_array<m65c02_device, 2> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_attr;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_m68k_shared_ram;
	required_shared_ptr<uint16_t> m_sprite_ram;
	int m_sprite16;
	int m_sprite32;
	int m_bg;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap[2];
	int32_t m_bg_scroll_x[2];
	int32_t m_bg_scroll_y[2];
	bitmap_ind16 m_temp_bitmap;
	std::unique_ptr<uint16_t[]> m_sprite_ram_buffered;
	std::unique_ptr<uint8_t[]> m_decoded_16;
	std::unique_ptr<uint8_t[]> m_decoded_32;
	int m_is_mask_spr[1024/16];
	DECLARE_READ8_MEMBER(m68k_shared_r);
	DECLARE_WRITE8_MEMBER(m68k_shared_w);
	DECLARE_WRITE8_MEMBER(m6809_irq_enable_w);
	DECLARE_WRITE8_MEMBER(m6809_irq_disable_w);
	DECLARE_WRITE16_MEMBER(m68k_irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_disable_w);
	DECLARE_READ8_MEMBER(dsw0_r);
	DECLARE_READ8_MEMBER(dsw1_r);
	DECLARE_READ8_MEMBER(input0_r);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_WRITE8_MEMBER(tceptor_tile_ram_w);
	DECLARE_WRITE8_MEMBER(tceptor_tile_attr_w);
	DECLARE_WRITE8_MEMBER(tceptor_bg_ram_w);
	DECLARE_WRITE8_MEMBER(tceptor_bg_scroll_w);
	DECLARE_WRITE8_MEMBER(tceptor2_shutter_w);
	void tile_mark_dirty(int offset);

	required_device<namco_c45_road_device> m_c45_road;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	output_finder<> m_shutter;

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void tceptor_palette(palette_device &palette);
	uint32_t screen_update_tceptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_tceptor);
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

	void m6502_a_map(address_map &map);
	void m6502_b_map(address_map &map);
	void m6809_map(address_map &map);
	void m68k_map(address_map &map);
	void mcu_io_map(address_map &map);
	void mcu_map(address_map &map);
};

#endif // MAME_INCLUDES_TCEPTOR_H
