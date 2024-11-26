// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_JALECO_TETRISP2_H
#define MAME_JALECO_TETRISP2_H

#pragma once

#include "jaleco_ms32_sysctrl.h"
#include "jaleco_vj_pc.h"
#include "ms32_sprite.h"

#include "machine/gen_latch.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class tetrisp2_state : public driver_device
{
public:
	tetrisp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sysctrl(*this, "sysctrl")
		, m_sprite(*this, "sprite")
		, m_screen(*this, "screen")
		, m_spriteram(*this, "spriteram")
		, m_vram_fg(*this, "vram_fg")
		, m_vram_bg(*this, "vram_bg")
		, m_vram_rot(*this, "vram_rot")
		, m_nvram(*this, "nvram")
		, m_scroll_fg(*this, "scroll_fg")
		, m_scroll_bg(*this, "scroll_bg")
		, m_rotregs(*this, "rotregs")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram")
		, m_io_system(*this, "SYSTEM")
		, m_leds(*this, "led%u", 0U)
	{ }

	void tetrisp2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD { m_leds.resolve(); }
	virtual void video_start() override ATTR_COLD;

	void setup_main_sysctrl(machine_config &config, const XTAL clock) ATTR_COLD;
	void setup_main_sprite(machine_config &config, const XTAL clock) ATTR_COLD;

	void flipscreen_w(int state);
	void timer_irq_w(int state);
	void vblank_irq_w(int state);
	void field_irq_w(int state);
	void sound_reset_line_w(int state);

	u16 tetrisp2_ip_1_word_r();
	void tetrisp2_coincounter_w(u16 data);
	u16 tetrisp2_nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 priority_r(offs_t offset);
	void vram_bg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vram_fg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vram_rot_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_rot);

	u32 screen_update_tetrisp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tetrisp2_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<jaleco_ms32_sysctrl_device> m_sysctrl;
	required_device<ms32_sprite_device> m_sprite;
	required_device<screen_device> m_screen;

	required_shared_ptr<u16> m_spriteram;

	required_shared_ptr<u16> m_vram_fg;
	required_shared_ptr<u16> m_vram_bg;
	required_shared_ptr<u16> m_vram_rot;
	required_shared_ptr<u16> m_nvram;
	required_shared_ptr<u16> m_scroll_fg;
	required_shared_ptr<u16> m_scroll_bg;
	required_shared_ptr<u16> m_rotregs;
	std::unique_ptr<u8[]> m_priority;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<u16> m_paletteram;

	optional_ioport m_io_system;
	output_finder<45> m_leds;

	s32 m_rot_ofsx = 0, m_rot_ofsy = 0;

	tilemap_t *m_tilemap_bg = nullptr;
	tilemap_t *m_tilemap_fg = nullptr;
	tilemap_t *m_tilemap_rot = nullptr;
};

// With camera, printer, OKI MSM6295 sound subsystem
class nndmseal_state : public tetrisp2_state
{
public:
	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }

	nndmseal_state(const machine_config &mconfig, device_type type, const char *tag)
		: tetrisp2_state(mconfig, type, tag)
		, m_okibank(*this, "okibank%u", 0U)
	{ }

	void nndmseal(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void nndmseal_sound_bank_w(u8 data);
	void nndmseal_coincounter_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void nndmseal_b20000_w(u16 data);

	void nndmseal_map(address_map &map) ATTR_COLD;
	void nndmseal_oki_map(address_map &map) ATTR_COLD;

	required_memory_bank_array<2> m_okibank;

	u8 m_bank_lo;
	u8 m_bank_hi;
};

// with ADPCM bankswitching and alternative layer size
class rockn_state : public tetrisp2_state
{
public:
	rockn_state(const machine_config &mconfig, device_type type, const char *tag)
		: tetrisp2_state(mconfig, type, tag)
		, m_spriteram2(*this, "spriteram2")
		, m_ymzbank(*this, "ymzbank_%u", 0U)
	{ }

	void rockn(machine_config &config) ATTR_COLD;
	void rockn2(machine_config &config) ATTR_COLD;

	void init_rockn() ATTR_COLD;
	void init_rockn2() ATTR_COLD;
	void init_rockn3() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	u16 rockn_adpcmbank_r();
	void rockn_adpcmbank_w(u16 data);
	void rockn2_adpcmbank_w(u16 data);
	u16 rockn_soundvolume_r();
	void rockn_soundvolume_w(u16 data);
	u16 rockn_nvram_r(offs_t offset);

	u32 screen_update_rockntread(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void rockn1_map(address_map &map) ATTR_COLD;
	void rockn1_ymz_map(address_map &map) ATTR_COLD;
	void rockn2_map(address_map &map) ATTR_COLD;
	void rockn2_ymz_map(address_map &map) ATTR_COLD;

	optional_shared_ptr<u16> m_spriteram2;

	optional_memory_bank_array<3> m_ymzbank;

	u16 m_rockn_protectdata = 0;
	u16 m_rockn_adpcmbank = 0;
	u16 m_rockn_soundvolume = 0;
};

class rocknms_state : public rockn_state
{
public:
	rocknms_state(const machine_config &mconfig, device_type type, const char *tag)
		: rockn_state(mconfig, type, tag)
		, m_subcpu(*this, "sub")
		, m_sub_sysctrl(*this, "sub_sysctrl")
		, m_sub_screen(*this, "sub_screen")
		, m_sub_sprite(*this, "sub_sprite")
		, m_sub_priority(*this, "sub_priority")
		, m_sub_vram_rot(*this, "sub_vram_rot")
		, m_sub_vram_fg(*this, "sub_vram_fg")
		, m_sub_vram_bg(*this, "sub_vram_bg")
		, m_sub_scroll_fg(*this, "sub_scroll_fg")
		, m_sub_scroll_bg(*this, "sub_scroll_bg")
		, m_sub_rotregs(*this, "sub_rotregs")
		, m_sub_palette(*this, "sub_palette")
		, m_sub_paletteram(*this, "sub_paletteram")
		, m_sub_gfxdecode(*this, "sub_gfxdecode")
	{ }

	void rocknms(machine_config &config) ATTR_COLD;

	ioport_value main2sub_status_r();

	void init_rocknms() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_subcpu;
	required_device<jaleco_ms32_sysctrl_device> m_sub_sysctrl;
	required_device<screen_device> m_sub_screen;
	required_device<ms32_sprite_device> m_sub_sprite;
	required_shared_ptr<u16> m_sub_priority;
	required_shared_ptr<u16> m_sub_vram_rot;
	required_shared_ptr<u16> m_sub_vram_fg;
	required_shared_ptr<u16> m_sub_vram_bg;
	required_shared_ptr<u16> m_sub_scroll_fg;
	required_shared_ptr<u16> m_sub_scroll_bg;
	required_shared_ptr<u16> m_sub_rotregs;
	required_device<palette_device> m_sub_palette;
	required_shared_ptr<u16> m_sub_paletteram;
	required_device<gfxdecode_device> m_sub_gfxdecode;

	u32 screen_update_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_bottom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rocknms_main_map(address_map &map) ATTR_COLD;
	void rocknms_sub_map(address_map &map) ATTR_COLD;

	u16 main2sub_r();
	void main2sub_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub2main_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub_palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub_priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub_vram_bg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub_vram_fg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub_vram_rot_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info_sub_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_sub_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_sub_rot);

	void sub_flipscreen_w(int state);
	void sub_timer_irq_w(int state);
	void sub_vblank_irq_w(int state);
	void sub_field_irq_w(int state);
	void sub_sound_reset_line_w(int state);

	u16 m_main2sub = 0;
	u16 m_sub2main = 0;

	tilemap_t *m_tilemap_sub_bg = nullptr;
	tilemap_t *m_tilemap_sub_fg = nullptr;
	tilemap_t *m_tilemap_sub_rot = nullptr;
};

class stepstag_state : public rockn_state
{
public:
	stepstag_state(const machine_config &mconfig, device_type type, const char *tag)
		: rockn_state(mconfig, type, tag)
		, m_subcpu(*this, "sub")
		, m_vj_sprite_l(*this, "sprite_l")
		, m_vj_sprite_m(*this, "sprite_m")
		, m_vj_sprite_r(*this, "sprite_r")
		, m_spriteram1(*this, "spriteram1")
		, m_spriteram3(*this, "spriteram3")
		, m_vj_palette_l(*this, "lpalette")
		, m_vj_palette_m(*this, "mpalette")
		, m_vj_palette_r(*this, "rpalette")
		, m_vj_paletteram_l(*this, "paletteram1")
		, m_vj_paletteram_m(*this, "paletteram2")
		, m_vj_paletteram_r(*this, "paletteram3")
		, m_soundlatch(*this, "soundlatch")
		, m_jaleco_vj_pc(*this, "jaleco_vj_pc")
		, m_soundvr(*this, "SOUND_VR%u", 1)
		, m_rscreen(*this, "rscreen")
		, m_io_coins(*this, "COINS")
	{ }

	void stepstag(machine_config &config) ATTR_COLD;
	void vjdash(machine_config &config) ATTR_COLD;

	DECLARE_VIDEO_START(stepstag);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 stepstag_coins_r();
	void stepstag_b00000_w(u16 data);
	void stepstag_b20000_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 stepstag_sprite_status_status_r();
	u16 unknown_read_0xffff00();
	void stepstag_soundlatch_word_w(u16 data);
	void stepstag_neon_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_step_leds_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_button_leds_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void palette_left_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void palette_mid_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void palette_right_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void stepstag_spriteram1_updated_w(u16 data);
	void stepstag_spriteram2_updated_w(u16 data);
	void stepstag_spriteram3_updated_w(u16 data);
	void adv7176a_w(u16 data);

	u16 stepstag_soundvolume_r();

	u32 screen_update_stepstag_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_stepstag_mid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_stepstag_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 screen_update_vjdash_main(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_vjdash_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_vjdash_mid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_vjdash_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 screen_update_nop(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void stepstag_map(address_map &map) ATTR_COLD;
	void stepstag_sub_map(address_map &map) ATTR_COLD;
	void vjdash_map(address_map &map) ATTR_COLD;

	void field_cb(int state);
	void setup_non_sysctrl_screen(machine_config &config, screen_device *screen, const XTAL xtal);

	void convert_yuv422_to_rgb888(palette_device *paldev, u16 *palram,u32 offset);

	required_device<cpu_device> m_subcpu;
	optional_device<ms32_sprite_device> m_vj_sprite_l;
	optional_device<ms32_sprite_device> m_vj_sprite_m;
	optional_device<ms32_sprite_device> m_vj_sprite_r;
	required_shared_ptr<u16> m_spriteram1;
	required_shared_ptr<u16> m_spriteram3;
	optional_device<palette_device> m_vj_palette_l;
	optional_device<palette_device> m_vj_palette_m;
	optional_device<palette_device> m_vj_palette_r;
	optional_shared_ptr<u16> m_vj_paletteram_l;
	optional_shared_ptr<u16> m_vj_paletteram_m;
	optional_shared_ptr<u16> m_vj_paletteram_r;
	required_device<generic_latch_16_device> m_soundlatch;
	required_device<jaleco_vj_pc_device> m_jaleco_vj_pc;
	optional_ioport_array<2> m_soundvr;
	required_device<screen_device> m_rscreen;

	required_ioport m_io_coins;

	std::unique_ptr<uint16_t[]> m_spriteram1_data;
	std::unique_ptr<uint16_t[]> m_spriteram2_data;
	std::unique_ptr<uint16_t[]> m_spriteram3_data;

	u16 m_vj_upload_idx = 0;
	bool m_vj_upload_fini = false;

	uint8_t m_adv7176a_sclock;
	uint8_t m_adv7176a_sdata;
	uint8_t m_adv7176a_state;
	uint8_t m_adv7176a_byte;
	uint8_t m_adv7176a_shift;
	uint16_t m_adv7176a_subaddr;
};

#endif // MAME_JALECO_TETRISP2_H
