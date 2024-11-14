// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco NA-1 System hardware

***************************************************************************/
#ifndef MAME_NAMCO_NAMCONA1_H
#define MAME_NAMCO_NAMCONA1_H

#pragma once

#include "machine/eeprompar.h"
#include "namcomcu.h"
#include "machine/timer.h"
#include "machine/msm6242.h"
#include "sound/c140.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class namcona1_state : public driver_device
{
public:
	namcona1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_c219(*this, "c219"),
		m_muxed_inputs(*this, { { "P4", "DSW", "P1", "P2" } }),
		m_io_p3(*this, "P3"),
		m_workram(*this, "workram"),
		m_vreg(*this, "vreg"),
		m_paletteram(*this, "paletteram"),
		m_cgram(*this, "cgram"),
		m_videoram(*this, "videoram"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_prgrom(*this, "maincpu"),
		m_maskrom(*this, "maskrom"),
		m_scan_timer(nullptr)
	{ }

	void namcona_base(machine_config &config);
	void c69(machine_config &config);
	void namcona1(machine_config &config);

	void init_bkrtmaq();
	void init_fa();
	void init_cgangpzl();
	void init_tinklpit();
	void init_swcourt();
	void init_exvania();
	void init_emeraldj();
	void init_swcourtb();

	void namcona1_mcu_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override;

	TIMER_CALLBACK_MEMBER(set_scanline_interrupt);
	void scanline_interrupt(int scanline);

	u16 custom_key_r(offs_t offset);
	void custom_key_w(u16 data);
	void vreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 mcu_mailbox_r(offs_t offset);
	void mcu_mailbox_w_68k(offs_t offset, u16 data, u16 mem_mask = ~0);
	void mcu_mailbox_w_mcu(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 na1mcu_shared_r(offs_t offset);
	void na1mcu_shared_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 port4_r();
	void port4_w(u8 data);
	u8 port5_r();
	void port5_w(u8 data);
	u8 port6_r();
	void port6_w(u8 data);
	u8 port7_r();
	void port7_w(u8 data);
	u8 port8_r();
	void port8_w(u8 data);
	template <int Bit> u16 portana_r();
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void paletteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 gfxram_r(offs_t offset);
	void gfxram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void namcona1_main_map(address_map &map) ATTR_COLD;
	void namcona1_c219_map(address_map &map) ATTR_COLD;

	enum
	{
		NAMCO_CGANGPZL,
		NAMCO_EMERALDA,
		NAMCO_KNCKHEAD,
		NAMCO_BKRTMAQ,
		NAMCO_EXVANIA,
		NAMCO_QUIZTOU,
		NAMCO_SWCOURT,
		NAMCO_TINKLPIT,
		NAMCO_NUMANATH,
		NAMCO_FA,
		NAMCO_XDAY2,
		NAMCO_SWCOURTB
	};

	int m_gametype;

	required_device<cpu_device> m_maincpu;
	required_device<m37710_cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<c219_device> m_c219;

	required_ioport_array<4> m_muxed_inputs;
	required_ioport          m_io_p3;

	required_shared_ptr<u16> m_workram;
	required_shared_ptr<u16> m_vreg;
	required_shared_ptr<u16> m_paletteram;
	required_shared_ptr<u16> m_cgram;
	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u16> m_scroll;
	required_shared_ptr<u16> m_spriteram;

	required_region_ptr<u16> m_prgrom;
	required_region_ptr<u16> m_maskrom;

	emu_timer * m_scan_timer;
	// this has to be u8 to be in the right byte order for the tilemap system
	std::vector<u8> m_shaperam;

	int m_enable_interrupts;
	u16 m_count;
	u32 m_keyval;
	u16 m_mcu_mailbox[8];
	u8 m_mcu_port4;
	u8 m_mcu_port5;
	u8 m_mcu_port6;
	u8 m_mcu_port8;
	tilemap_t *m_bg_tilemap[4+1];
	int m_palette_is_dirty;

	void simulate_mcu();
	void write_version_info();
	int transfer_dword(u32 dest, u32 source);

	void blit();
	void update_palette(int offset);
	void pdraw_tile(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, u32 code, u32 color,
		int sx, int sy, bool flipx, bool flipy, u8 priority, bool bShadow, bool bOpaque, u8 gfx_region);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask);
	void tilemap_get_info(tile_data &tileinfo, int tile_index, const u16 *tilemap_videoram, bool use_4bpp_gfx);
	void blit_setup(int format, int *bytes_per_row, int *pitch, int mode);
	void draw_pixel_line(const rectangle &cliprect, u16 *pDest, u8 *pPri, u16 *pSource, const pen_t *paldata);
	bool screen_enabled(const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(tilemap_get_info0);
	TILE_GET_INFO_MEMBER(tilemap_get_info1);
	TILE_GET_INFO_MEMBER(tilemap_get_info2);
	TILE_GET_INFO_MEMBER(tilemap_get_info3);
	TILE_GET_INFO_MEMBER(roz_get_info);
};

class namcona2_state : public namcona1_state
{
public:
	namcona2_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcona1_state(mconfig, type, tag)
	{}

	void c70(machine_config &config);
	void namcona2(machine_config &config);
	void zelos(machine_config &config);

	void init_knckhead();
	void init_emeralda();
	void init_numanath();
	void init_quiztou();
	void init_zelos();

private:
	u16 m_zelos_ctrl = 0;

	void zelos_ctrl_w(u16 data);
	void zelos_main_map(address_map &map) ATTR_COLD;
};

class xday2_namcona2_state : public namcona2_state
{
public:
	xday2_namcona2_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcona2_state(mconfig, type, tag),
		m_rtc(*this, "rtc")
	{}

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	void xday2(machine_config &config);

	void init_xday2();

private:
	required_device <msm6242_device> m_rtc;

	u8 printer_r();
	void printer_w(u8 data);

	void xday2_main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_NAMCONA1_H
