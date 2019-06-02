// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco NA-1 System hardware

***************************************************************************/
#ifndef MAME_INCLUDES_NAMCONA1_H
#define MAME_INCLUDES_NAMCONA1_H

#pragma once

#include "machine/eeprompar.h"
#include "machine/namcomcu.h"
#include "machine/timer.h"
#include "machine/msm6242.h"
#include "sound/c140.h"
#include "emupal.h"
#include "screen.h"


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
		m_c140(*this, "c140"),
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

	void namcona1_mcu_map(address_map &map);

protected:
	DECLARE_READ16_MEMBER(custom_key_r);
	DECLARE_WRITE16_MEMBER(custom_key_w);
	DECLARE_WRITE16_MEMBER(vreg_w);
	DECLARE_READ16_MEMBER(mcu_mailbox_r);
	DECLARE_WRITE16_MEMBER(mcu_mailbox_w_68k);
	DECLARE_WRITE16_MEMBER(mcu_mailbox_w_mcu);
	DECLARE_READ16_MEMBER(na1mcu_shared_r);
	DECLARE_WRITE16_MEMBER(na1mcu_shared_w);
	DECLARE_READ8_MEMBER(port4_r);
	DECLARE_WRITE8_MEMBER(port4_w);
	DECLARE_READ8_MEMBER(port5_r);
	DECLARE_WRITE8_MEMBER(port5_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(port7_r);
	DECLARE_WRITE8_MEMBER(port7_w);
	DECLARE_READ8_MEMBER(port8_r);
	DECLARE_WRITE8_MEMBER(port8_w);
	template <int Bit> uint16_t portana_r();
	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(paletteram_w);
	DECLARE_READ16_MEMBER(gfxram_r);
	DECLARE_WRITE16_MEMBER(gfxram_w);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void scanline_interrupt(int scanline);

	void namcona1_main_map(address_map &map);
	void namcona1_c140_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

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

	enum
	{
		TIMER_SCANLINE
	};

	int m_gametype;

	required_device<cpu_device> m_maincpu;
	required_device<m37710_cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<c140_device> m_c140;

	required_ioport_array<4> m_muxed_inputs;
	required_ioport          m_io_p3;

	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_vreg;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_cgram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_spriteram;

	required_region_ptr<uint16_t> m_prgrom;
	required_region_ptr<uint16_t> m_maskrom;

	emu_timer * m_scan_timer;
	// this has to be uint8_t to be in the right byte order for the tilemap system
	std::vector<uint8_t> m_shaperam;

	int m_mEnableInterrupts;
	uint16_t m_count;
	uint32_t m_keyval;
	uint16_t m_mcu_mailbox[8];
	uint8_t m_mcu_port4;
	uint8_t m_mcu_port5;
	uint8_t m_mcu_port6;
	uint8_t m_mcu_port8;
	tilemap_t *m_bg_tilemap[4+1];
	int m_palette_is_dirty;

	void simulate_mcu();
	void write_version_info();
	int transfer_dword(uint32_t dest, uint32_t source);

	void blit();
	void UpdatePalette(int offset);
	void pdraw_tile( screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, uint32_t code, int color,
		int sx, int sy, int flipx, int flipy, int priority, int bShadow, int bOpaque, int gfx_region );
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask );
	void tilemap_get_info(tile_data &tileinfo, int tile_index, const uint16_t *tilemap_videoram, bool use_4bpp_gfx);
	void blit_setup( int format, int *bytes_per_row, int *pitch, int mode );
	void draw_pixel_line( const rectangle &cliprect, uint16_t *pDest, uint8_t *pPri, uint16_t *pSource, const pen_t *paldata );
	bool screen_enabled( const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(tilemap_get_info0);
	TILE_GET_INFO_MEMBER(tilemap_get_info1);
	TILE_GET_INFO_MEMBER(tilemap_get_info2);
	TILE_GET_INFO_MEMBER(tilemap_get_info3);
	TILE_GET_INFO_MEMBER(roz_get_info);

	void postload();
};

class namcona2_state : public namcona1_state
{
public:
	namcona2_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcona1_state(mconfig, type, tag)
	{}

	void c70(machine_config &config);
	void namcona2(machine_config &config);

	void init_knckhead();
	void init_emeralda();
	void init_numanath();
	void init_quiztou();
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

	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(printer_w);

	void xday2_main_map(address_map &map);
};

#endif // MAME_INCLUDES_NAMCONA1_H
