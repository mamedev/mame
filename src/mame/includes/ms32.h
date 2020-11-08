// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest
#ifndef MAME_INCLUDES_MS32_H
#define MAME_INCLUDES_MS32_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/jaleco_ms32_sysctrl.h"
#include "video/ms32_sprite.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ms32_state : public driver_device
{
public:
	ms32_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sysctrl(*this, "sysctrl"),
		m_sprite(*this, "sprite"),
		m_soundlatch(*this, "soundlatch"),
		m_sprite_ctrl(*this, "sprite_ctrl"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_roz_ctrl(*this, "roz_ctrl"),
		m_tx_scroll(*this, "tx_scroll"),
		m_bg_scroll(*this, "bg_scroll"),
		m_mahjong_input_select(*this, "mahjong_select"),
		m_priram(*this, "priram",  0x2000, ENDIANNESS_LITTLE),
		m_palram(*this, "palram", 0x20000, ENDIANNESS_LITTLE),
		m_rozram(*this, "rozram", 0x10000, ENDIANNESS_LITTLE),
		m_lineram(*this, "lineram", 0x1000, ENDIANNESS_LITTLE),
		m_sprram(*this, "sprram", 0x10000, ENDIANNESS_LITTLE),
		m_txram(*this, "txram", 0x4000, ENDIANNESS_LITTLE),
		m_bgram(*this, "bgram", 0x4000, ENDIANNESS_LITTLE),
		m_f1superb_extraram(*this, "f1sb_extraram", 0x10000, ENDIANNESS_LITTLE),
		m_z80bank(*this, "z80bank%u", 1)
	{ }

	void ms32(machine_config &config);
	void f1superb(machine_config &config);

	void init_ss92047_01();
	void init_ss91022_10();
	void init_kirarast();
	void init_suchie2();
	void init_ss92048_01();
	void init_bnstars();
	void init_f1superb();
	void init_ss92046_01();

	IRQ_CALLBACK_MEMBER(irq_callback);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_ctrl_r);

protected:

	void configure_banks();

	void ms32_snd_bank_w(u8 data);

	u8 latch_r();
	void to_main_w(u8 data);
	u32 sound_result_r();
	void sound_command_w(u32 data);
	void sound_reset_w(u32 data);

	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<jaleco_ms32_sysctrl_device> m_sysctrl;
	required_device<ms32_sprite_device> m_sprite;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_shared_ptr<u32> m_sprite_ctrl;

	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(timer_irq_w);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq_w);
	DECLARE_WRITE_LINE_MEMBER(field_irq_w);
	DECLARE_WRITE_LINE_MEMBER(sound_reset_line_w);
	void irq_raise(int level);

	optional_device<screen_device> m_screen;
private:
	optional_shared_ptr<u32> m_mainram;
	optional_shared_ptr<u32> m_roz_ctrl;
	optional_shared_ptr<u32> m_tx_scroll;
	optional_shared_ptr<u32> m_bg_scroll;
	optional_shared_ptr<u32> m_mahjong_input_select;
	memory_share_creator<u8> m_priram;
	memory_share_creator<u16> m_palram;
	memory_share_creator<u16> m_rozram;
	memory_share_creator<u16> m_lineram;
	memory_share_creator<u16> m_sprram;
	memory_share_creator<u16> m_txram;
	memory_share_creator<u16> m_bgram;
	memory_share_creator<u16> m_f1superb_extraram;

	optional_memory_bank_array<2> m_z80bank;
	std::unique_ptr<u8[]> m_nvram_8;
	std::unique_ptr<u16[]> m_sprram_buffer;
	u32 m_to_main;
	u16 m_irqreq;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_roz_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_alt;
	u32 m_tilemaplayoutcontrol;
	tilemap_t* m_extra_tilemap;
	bitmap_ind16 m_temp_bitmap_tilemaps;
	bitmap_ind16 m_temp_bitmap_sprites;
	bitmap_ind8 m_temp_bitmap_sprites_pri;
	u32 m_brt[4];
	int m_brt_r;
	int m_brt_g;
	int m_brt_b;
	u32 ms32_read_inputs3();
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
	void ms32_extra_w16(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ms32_extra_r16(offs_t offset);
	void ms32_irq2_guess_w(u32 data);
	void ms32_irq5_guess_w(u32 data);
	void ms32_brightness_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void coin_counter_w(u32 data);
	void init_ms32_common();

	TILE_GET_INFO_MEMBER(get_ms32_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_roz_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_extra_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(f1superb);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void irq_init();
	void update_color(int color);
	void draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_pri, const rectangle &cliprect, u16 *sprram_top);
	void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,int priority);
	void f1superb_map(address_map &map);
	void ms32_map(address_map &map);
	void ms32_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MS32_H
