// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
#ifndef MAME_INCLUDES_TATSUMI_H
#define MAME_INCLUDES_TATSUMI_H

#pragma once

#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "cpu/m68000/m68000.h"
#include "machine/cxd1095.h"
#include "machine/gen_latch.h"
#include "emupal.h"

class tatsumi_state : public driver_device
{
public:
	tatsumi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_ym2151(*this, "ymsnd")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_sharedram(*this, "sharedram")
		, m_sprite_control_ram(*this, "obj_ctrl_ram")
		, m_spriteram(*this, "spriteram")
		, m_mainregion(*this, "master_rom")
		, m_subregion(*this, "slave_rom")
	{ }

	DECLARE_WRITE8_MEMBER(hd6445_crt_w);
	INTERRUPT_GEN_MEMBER(v30_interrupt);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<m68000_base_device> m_subcpu;
	optional_device<ym2151_device> m_ym2151;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint16_t> m_videoram;
	optional_shared_ptr<uint16_t> m_sharedram;
	required_shared_ptr<uint16_t> m_sprite_control_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_memory_region m_mainregion;
	required_memory_region m_subregion;

	uint8_t *m_rom_sprite_lookup[2];
	uint8_t *m_rom_clut[2];
	uint16_t m_control_word;
	uint8_t m_last_control;
	tilemap_t *m_tx_layer;
	bitmap_rgb32 m_temp_bitmap;
	std::unique_ptr<uint8_t[]> m_shadow_pen_array;
	DECLARE_WRITE16_MEMBER(text_w);
	DECLARE_READ16_MEMBER(tatsumi_v30_68000_r);
	DECLARE_WRITE16_MEMBER(tatsumi_v30_68000_w);
	DECLARE_READ16_MEMBER(tatsumi_sprite_control_r);
	DECLARE_WRITE16_MEMBER(tatsumi_sprite_control_w);

	DECLARE_READ8_MEMBER(tatsumi_hack_ym2151_r);

	void tatsumi_reset();
	template<class _BitmapClass> void draw_sprites(_BitmapClass &bitmap, const rectangle &cliprect, int write_priority_only, int rambank);
	template<class _BitmapClass> inline void roundupt_drawgfxzoomrotate( _BitmapClass &dest_bmp, const rectangle &clip,
		gfx_element *gfx, uint32_t code,uint32_t color,int flipx,int flipy,uint32_t ssx,uint32_t ssy,
		int scalex, int scaley, int rotate, int write_priority_only );
	void update_cluts(int fake_palette_offset, int object_base, int length);

	uint8_t m_hd6445_reg[64];
	void apply_shadow_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &shadow_bitmap, uint8_t xor_output);

	uint8_t m_hd6445_address;
};

class apache3_state : public tatsumi_state
{
public:
	apache3_state(const machine_config &mconfig, device_type type, const char *tag)
		: tatsumi_state(mconfig, type, tag)
		, m_subcpu2(*this, "sub2")
		, m_apache3_g_ram(*this, "apache3_g_ram")
		, m_apache3_z80_ram(*this, "apache3_z80_ram")
		, m_apache3_prom(*this, "proms")
		, m_vr1(*this, "VR1")
	{
	}

	void apache3(machine_config &config);

	void init_apache3();

private:
	DECLARE_READ16_MEMBER(apache3_bank_r);
	DECLARE_WRITE16_MEMBER(apache3_bank_w);
	DECLARE_WRITE16_MEMBER(apache3_z80_ctrl_w);
	DECLARE_READ16_MEMBER(apache3_v30_v20_r);
	DECLARE_WRITE16_MEMBER(apache3_v30_v20_w);
	DECLARE_READ16_MEMBER(apache3_z80_r);
	DECLARE_WRITE16_MEMBER(apache3_z80_w);
	DECLARE_READ8_MEMBER(apache3_vr1_r);
	DECLARE_WRITE16_MEMBER(apache3_rotate_w);
	DECLARE_WRITE16_MEMBER(apache3_road_z_w);
	DECLARE_WRITE8_MEMBER(apache3_road_x_w);

	DECLARE_MACHINE_RESET(apache3);
	DECLARE_VIDEO_START(apache3);
	uint32_t screen_update_apache3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(apache3_68000_reset);

	void apache3_68000_map(address_map &map);
	void apache3_v20_map(address_map &map);
	void apache3_v30_map(address_map &map);
	void apache3_z80_map(address_map &map);

	void draw_sky(bitmap_rgb32 &bitmap, const rectangle &cliprect, int palette_base, int start_offset);
	void draw_ground(bitmap_rgb32 &dst, const rectangle &cliprect);

	required_device<cpu_device> m_subcpu2;

	required_shared_ptr<uint16_t> m_apache3_g_ram;
	required_shared_ptr<uint8_t> m_apache3_z80_ram;
	required_region_ptr<uint8_t> m_apache3_prom;

	required_ioport m_vr1;

	uint16_t m_apache3_rotate_ctrl[12];
	int m_apache3_rot_idx;
	std::unique_ptr<uint8_t[]> m_apache3_road_x_ram;
	uint8_t m_apache3_road_z;
};

class roundup5_state : public tatsumi_state
{
public:
	roundup5_state(const machine_config &mconfig, device_type type, const char *tag)
		: tatsumi_state(mconfig, type, tag)
		, m_vregs(*this, "vregs")
		, m_bg_scrollx(*this, "bg_scrollx")
		, m_bg_scrolly(*this, "bg_scrolly")
		, m_road_ctrl_ram(*this, "road_ctrl_ram")
		, m_road_pixel_ram(*this, "road_pixel_ram")
		, m_road_color_ram(*this, "road_color_ram")
		, m_road_yclip(*this, "road_yclip")
		, m_road_vregs(*this, "road_vregs")
	{
	}

	void roundup5(machine_config &config);

	void init_roundup5();

private:
	DECLARE_READ16_MEMBER(roundup_v30_z80_r);
	DECLARE_WRITE16_MEMBER(roundup_v30_z80_w);
	DECLARE_WRITE16_MEMBER(roundup5_control_w);
	DECLARE_WRITE16_MEMBER(road_vregs_w);
	DECLARE_READ8_MEMBER(gfxdata_r);
	DECLARE_WRITE8_MEMBER(gfxdata_w);
	DECLARE_WRITE8_MEMBER(output_w);

	DECLARE_VIDEO_START(roundup5);
	uint32_t screen_update_roundup5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void roundup5_68000_map(address_map &map);
	void roundup5_v30_map(address_map &map);
	void roundup5_z80_map(address_map &map);

//  virtual void machine_reset() override;

	void draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_landscape(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t type);

	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_bg_scrollx;
	required_shared_ptr<uint16_t> m_bg_scrolly;
	required_shared_ptr<uint16_t> m_road_ctrl_ram;
	required_shared_ptr<uint16_t> m_road_pixel_ram;
	required_shared_ptr<uint16_t> m_road_color_ram;
	required_shared_ptr<uint16_t> m_road_yclip;
	required_shared_ptr<uint16_t> m_road_vregs;

	std::unique_ptr<uint8_t[]> m_tx_gfxram;
	std::unique_ptr<uint8_t[]> m_bg_gfxram;
};

class cyclwarr_state : public tatsumi_state
{
public:
	cyclwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: tatsumi_state(mconfig, type, tag)
		, m_soundlatch(*this, "soundlatch")
		, m_master_ram(*this, "master_ram")
		, m_slave_ram(*this, "slave_ram")
		, m_cyclwarr_videoram(*this, "cw_videoram%u", 0U)
		, m_cyclwarr_tileclut(*this, "cw_tileclut")
	{
	}

	void cyclwarr(machine_config &config);
	void bigfight(machine_config &config);

	void init_cyclwarr();

protected:
	virtual void machine_reset() override;

private:
	DECLARE_READ16_MEMBER(cyclwarr_sprite_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_sprite_w);
	DECLARE_WRITE16_MEMBER(video_config_w);
	DECLARE_WRITE16_MEMBER(bigfight_a40000_w);
	DECLARE_WRITE16_MEMBER(mixing_control_w);
	DECLARE_WRITE8_MEMBER(cyclwarr_control_w);
	DECLARE_WRITE8_MEMBER(cyclwarr_sound_w);
	DECLARE_WRITE16_MEMBER(output_w);
	DECLARE_READ8_MEMBER(oki_status_xor_r);

	template<int Bank> DECLARE_READ16_MEMBER(cyclwarr_videoram_r);
	template<int Bank> DECLARE_WRITE16_MEMBER(cyclwarr_videoram_w);

	template<int Bank> TILE_GET_INFO_MEMBER(get_tile_info_bigfight);
	template<int Bank> TILE_GET_INFO_MEMBER(get_tile_info_cyclwarr_road);
	DECLARE_VIDEO_START(cyclwarr);
	DECLARE_VIDEO_START(bigfight);
	uint32_t screen_update_cyclwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void common_map(address_map &map);
	void master_map(address_map &map);
	void slave_map(address_map &map);
	void sound_map(address_map &map);

	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_master_ram;
	required_shared_ptr<uint16_t> m_slave_ram;
	required_shared_ptr_array<uint16_t, 2> m_cyclwarr_videoram;
	required_region_ptr<uint8_t> m_cyclwarr_tileclut;

	std::vector<uint8_t> m_mask;
	tilemap_t *m_layer[4];

	uint16_t m_video_config[4];
	uint16_t m_mixing_control;
	uint16_t m_bigfight_a40000[2];
	uint16_t m_bigfight_bank;
	uint16_t m_bigfight_last_bank;
	uint16_t m_road_color_bank, m_prev_road_bank;
	uint16_t m_layer_page_size[4];
	bool m_layer1_can_be_road;

	void tile_expand();
	void draw_bg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *src, const uint16_t* scrollx, const uint16_t* scrolly, const uint16_t layer_page_size, bool is_road, int hi_priority);
	void draw_bg_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int hi_priority);
	void apply_highlight_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &highlight_bitmap);
};

#endif // MAME_INCLUDES_TATSUMI_H
