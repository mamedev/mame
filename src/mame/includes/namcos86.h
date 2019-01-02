// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_NAMCOS86_H
#define MAME_INCLUDES_NAMCOS86_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "machine/watchdog.h"
#include "sound/n63701x.h"
#include "sound/namco.h"
#include "emupal.h"

class namcos86_state : public driver_device
{
public:
	namcos86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_mcu(*this, "mcu")
		, m_watchdog(*this, "watchdog")
		, m_cus30(*this, "namco")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_63701x(*this, "namco2")
		, m_rthunder_videoram1(*this, "videoram1")
		, m_rthunder_videoram2(*this, "videoram2")
		, m_rthunder_spriteram(*this, "spriteram")
		, m_user1_ptr(*this, "user1")
		, m_leds(*this, "led%u", 0U)
	{ }

	void genpeitd(machine_config &config);
	void wndrmomo(machine_config &config);
	void roishtar(machine_config &config);
	void rthunder(machine_config &config);
	void hopmappy(machine_config &config);

	void init_namco86();

private:
	DECLARE_WRITE8_MEMBER(bankswitch1_w);
	DECLARE_WRITE8_MEMBER(bankswitch1_ext_w);
	DECLARE_WRITE8_MEMBER(bankswitch2_w);
	DECLARE_READ8_MEMBER(dsw0_r);
	DECLARE_READ8_MEMBER(dsw1_r);
	DECLARE_WRITE8_MEMBER(int_ack1_w);
	DECLARE_WRITE8_MEMBER(int_ack2_w);
	DECLARE_WRITE8_MEMBER(watchdog1_w);
	DECLARE_WRITE8_MEMBER(watchdog2_w);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(cus115_w);
	DECLARE_WRITE8_MEMBER(videoram1_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(tilebank_select_w);
	DECLARE_WRITE8_MEMBER(scroll0_w);
	DECLARE_WRITE8_MEMBER(scroll1_w);
	DECLARE_WRITE8_MEMBER(scroll2_w);
	DECLARE_WRITE8_MEMBER(scroll3_w);
	DECLARE_WRITE8_MEMBER(backcolor_w);
	DECLARE_WRITE8_MEMBER(spriteram_w);

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	TILE_GET_INFO_MEMBER(get_tile_info3);

	void namcos86_palette(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void scroll_w(address_space &space, int offset, int data, int layer);

	void common_mcu_map(address_map &map);
	void cpu1_map(address_map &map);
	void genpeitd_cpu2_map(address_map &map);
	void genpeitd_mcu_map(address_map &map);
	void hopmappy_cpu2_map(address_map &map);
	void hopmappy_mcu_map(address_map &map);
	void roishtar_cpu2_map(address_map &map);
	void roishtar_mcu_map(address_map &map);
	void rthunder_cpu2_map(address_map &map);
	void rthunder_mcu_map(address_map &map);
	void wndrmomo_cpu2_map(address_map &map);
	void wndrmomo_mcu_map(address_map &map);

	virtual void machine_start() override;
	virtual void video_start() override;

	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<hd63701_cpu_device> m_mcu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<namco_63701x_device> m_63701x;
	required_shared_ptr<uint8_t> m_rthunder_videoram1;
	required_shared_ptr<uint8_t> m_rthunder_videoram2;
	required_shared_ptr<uint8_t> m_rthunder_spriteram;
	optional_region_ptr<uint8_t> m_user1_ptr;
	output_finder<2> m_leds;

	uint8_t *m_spriteram;
	int m_wdog;
	int m_tilebank;
	int m_xscroll[4];
	int m_yscroll[4];
	tilemap_t *m_bg_tilemap[4];
	int m_backcolor;
	const uint8_t *m_tile_address_prom;
	int m_copy_sprites;

	inline void get_tile_info(tile_data &tileinfo,int tile_index,int layer,uint8_t *vram);
	void set_scroll(int layer);
};

#endif // MAME_INCLUDES_NAMCOS86_H
