// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#ifndef MAME_INCLUDES_TWIN16_H
#define MAME_INCLUDES_TWIN16_H

#pragma once

#include "video/bufsprite.h"
#include "sound/upd7759.h"
#include "sound/k007232.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class twin16_state : public driver_device
{
public:
	twin16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_gfxrombank(*this, "gfxrombank"),
		m_fixram(*this, "fixram"),
		m_videoram(*this, "videoram.%u", 0),
		m_zipram(*this, "zipram"),
		m_sprite_gfx_ram(*this, "sprite_gfx_ram"),
		m_gfxrom(*this, "gfxrom")
	{ }

	void devilw(machine_config &config);
	void miaj(machine_config &config);
	void twin16(machine_config &config);

	void init_twin16();

	DECLARE_WRITE8_MEMBER(volume_callback);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_twin16);
	uint32_t screen_update_twin16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_memory_bank m_gfxrombank;
	required_shared_ptr<uint16_t> m_fixram;
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	optional_shared_ptr<uint16_t> m_zipram;
	optional_shared_ptr<uint16_t> m_sprite_gfx_ram;
	required_region_ptr<uint16_t> m_gfxrom;

	uint16_t m_CPUA_register;
	uint16_t m_CPUB_register;
	bool m_is_fround;
	uint16_t m_sprite_buffer[0x800];
	emu_timer *m_sprite_timer;
	int m_sprite_busy;
	int m_need_process_spriteram;
	uint16_t m_scrollx[3];
	uint16_t m_scrolly[3];
	uint16_t m_video_register;
	tilemap_t *m_fixed_tmap;
	tilemap_t *m_scroll_tmap[2];

	DECLARE_WRITE16_MEMBER(CPUA_register_w);
	DECLARE_WRITE16_MEMBER(CPUB_register_w);

	DECLARE_READ16_MEMBER(sprite_status_r);
	DECLARE_WRITE16_MEMBER(video_register_w);
	DECLARE_WRITE16_MEMBER(fixram_w);
	DECLARE_WRITE16_MEMBER(videoram0_w);
	DECLARE_WRITE16_MEMBER(videoram1_w);
	DECLARE_WRITE16_MEMBER(zipram_w);

	DECLARE_READ8_MEMBER(upd_busy_r);
	DECLARE_WRITE8_MEMBER(upd_reset_w);
	DECLARE_WRITE8_MEMBER(upd_start_w);

	TILE_GET_INFO_MEMBER(fix_tile_info);
	TILE_GET_INFO_MEMBER(layer0_tile_info);
	TILE_GET_INFO_MEMBER(layer1_tile_info);

	TIMER_CALLBACK_MEMBER(sprite_tick);

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sub_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void tile_get_info(tile_data &tileinfo, uint16_t data, int color_base);

	int set_sprite_timer();
	void spriteram_process();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap );
	int spriteram_process_enable();
	void twin16_postload();
};

class fround_state : public twin16_state
{
public:
	fround_state(const machine_config &mconfig, device_type type, const char *tag)
		: twin16_state(mconfig, type, tag)
	{}

	void fround(machine_config &config);

	void init_fround();

private:
	DECLARE_WRITE16_MEMBER(fround_CPU_register_w);
	DECLARE_WRITE16_MEMBER(gfx_bank_w);

	void fround_map(address_map &map);

	virtual void video_start() override;
	virtual void tile_get_info(tile_data &tileinfo, uint16_t data, int color_base) override;

	uint8_t m_gfx_bank[4];
};

class cuebrickj_state : public twin16_state
{
public:
	cuebrickj_state(const machine_config &mconfig, device_type type, const char *tag)
		: twin16_state(mconfig, type, tag)
	{}

	void cuebrickj(machine_config &config);

	void init_cuebrickj();

private:
	DECLARE_WRITE8_MEMBER(nvram_bank_w);

	uint16_t m_nvram[0x400 * 0x20 / 2];
};

#endif // MAME_INCLUDES_TWIN16_H
