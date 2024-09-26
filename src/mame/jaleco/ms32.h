// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest
#ifndef MAME_JALECO_MS32_H
#define MAME_JALECO_MS32_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "jaleco_ms32_sysctrl.h"
#include "ms32_sprite.h"
#include "sound/ymf271.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ms32_base_state : public driver_device
{
public:
	ms32_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_z80bank(*this, "z80bank%u", 1)
		, m_sprite_ctrl(*this, "sprite_ctrl")
	{ }

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_memory_bank_array<2> m_z80bank;
	required_shared_ptr<u32> m_sprite_ctrl;

	void timer_irq_w(int state);
	void vblank_irq_w(int state);
	void field_irq_w(int state);
	void sound_ack_w(int state);
	void sound_reset_line_w(int state);

	void ms32_snd_bank_w(u8 data);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void configure_banks();
	u8 latch_r();
	void to_main_w(u8 data);
	u32 sound_result_r();
	void sound_command_w(u32 data);
	void irq_raise(int level, bool state);
	void irq_init();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void base_sound_map(address_map &map) ATTR_COLD;

private:
	u32 m_to_main = 0;
	u16 m_irqreq = 0;
};

class ms32_state : public ms32_base_state
{
public:
	ms32_state(const machine_config &mconfig, device_type type, const char *tag) :
		ms32_base_state(mconfig, type, tag)
		, m_sysctrl(*this, "sysctrl")
		, m_screen(*this, "screen")
		, m_sprite(*this, "sprite")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_ymf(*this, "ymf")
		, m_roz_ctrl(*this, "roz_ctrl")
		, m_tx_scroll(*this, "tx_scroll")
		, m_bg_scroll(*this, "bg_scroll")
		, m_mahjong_input_select(*this, "mahjong_select")
		, m_priram(*this, "priram",  0x2000, ENDIANNESS_LITTLE)
		, m_palram(*this, "palram", 0x20000, ENDIANNESS_LITTLE)
		, m_rozram(*this, "rozram", 0x10000, ENDIANNESS_LITTLE)
		, m_lineram(*this, "lineram", 0x1000, ENDIANNESS_LITTLE)
		, m_sprram(*this, "sprram", 0x10000, ENDIANNESS_LITTLE)
		, m_txram(*this, "txram", 0x4000, ENDIANNESS_LITTLE)
		, m_bgram(*this, "bgram", 0x4000, ENDIANNESS_LITTLE)
	{ }

	void ms32(machine_config &config);
	void ms32_invert_lines(machine_config &config);

	void init_ss92047_01();
	void init_ss91022_10();
	void init_kirarast();
	void init_suchie2();
	void init_ss92048_01();
	void init_bnstars();
	void init_ss92046_01();

	ioport_value mahjong_ctrl_r();

protected:
	required_device<jaleco_ms32_sysctrl_device> m_sysctrl;
	required_device<screen_device> m_screen;
	required_device<ms32_sprite_device> m_sprite;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ymf271_device> m_ymf;

	void flipscreen_w(int state);
	virtual void video_start() override ATTR_COLD;

	void ms32_map(address_map &map) ATTR_COLD;
	void ms32_sound_map(address_map &map) ATTR_COLD;

private:
	required_shared_ptr<u32> m_roz_ctrl;
	required_shared_ptr<u32> m_tx_scroll;
	required_shared_ptr<u32> m_bg_scroll;
	required_shared_ptr<u32> m_mahjong_input_select;
	memory_share_creator<u8> m_priram;
	memory_share_creator<u16> m_palram;
	memory_share_creator<u16> m_rozram;
	memory_share_creator<u16> m_lineram;
	memory_share_creator<u16> m_sprram;
	memory_share_creator<u16> m_txram;
	memory_share_creator<u16> m_bgram;

	std::unique_ptr<u8[]> m_nvram_8;

	std::unique_ptr<u16[]> m_sprram_buffer;
	size_t m_objectram_size;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_roz_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_alt;
	u32 m_tilemaplayoutcontrol;
	bitmap_ind16 m_temp_bitmap_tilemaps;
	bitmap_ind16 m_temp_bitmap_sprites;
	bitmap_ind8 m_temp_bitmap_sprites_pri;
	u32 m_brt[4];
	int m_brt_r;
	int m_brt_g;
	int m_brt_b;
	u8 ms32_nvram_r8(offs_t offset);
	void ms32_nvram_w8(offs_t offset, u8 data);
	u8 ms32_priram_r8(offs_t offset);
	void ms32_priram_w8(offs_t offset, u8 data);
	u16 ms32_palram_r16(offs_t offset);
	void ms32_palram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms32_rozram_r16(offs_t offset);
	void ms32_rozram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms32_lineram_r16(offs_t offset);
	void ms32_lineram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms32_sprram_r16(offs_t offset);
	void ms32_sprram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms32_txram_r16(offs_t offset);
	void ms32_txram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms32_bgram_r16(offs_t offset);
	void ms32_bgram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bgmode_w(u32 data);

	void ms32_brightness_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void coin_counter_w(u32 data);
	void init_ms32_common();

	TILE_GET_INFO_MEMBER(get_ms32_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_roz_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_bg_tile_info);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void update_color(int color);
	void draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_pri, const rectangle &cliprect, u16 *sprram_top);
	void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,int priority);
};

class ms32_f1superbattle_state : public ms32_state
{
public:
	ms32_f1superbattle_state(const machine_config &mconfig, device_type type, const char *tag) :
		ms32_state(mconfig, type, tag)
		, m_road_vram(*this, "road_vram", 0x10000, ENDIANNESS_LITTLE)
		// TODO: COPROs
	{}

	void f1superb(machine_config &config);
	void init_f1superb();

protected:
	virtual void video_start() override ATTR_COLD;
private:
	TILE_GET_INFO_MEMBER(get_ms32_extra_tile_info);

	void ms32_irq2_guess_w(u32 data);
	void ms32_irq5_guess_w(u32 data);

	memory_share_creator<u16> m_road_vram;

	void f1superb_map(address_map &map) ATTR_COLD;

	void road_vram_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 road_vram_r16(offs_t offset);

	u32 analog_r();

	tilemap_t* m_extra_tilemap;
};

#endif // MAME_JALECO_MS32_H
