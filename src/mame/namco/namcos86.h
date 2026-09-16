// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_NAMCOS86_H
#define MAME_NAMCO_NAMCOS86_H

#pragma once

#include "namco_cus4xtmap.h"
#include "namcos1_sprite.h"

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
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_tilegen(*this, "tilegen_%u", 0U)
		, m_63701x(*this, "namco2")
		, m_bankeddata_ptr(*this, "bankeddata")
		, m_mainbank(*this, "mainbank")
		, m_subbank(*this, "subbank")
		, m_io_dsw(*this, "DSW%c", 'A')
		, m_leds(*this, "led%u", 0U)
	{ }

	void genpeitd(machine_config &config) ATTR_COLD;
	void wndrmomo(machine_config &config) ATTR_COLD;
	void roishtar(machine_config &config) ATTR_COLD;
	void rthunder(machine_config &config) ATTR_COLD;
	void hopmappy(machine_config &config) ATTR_COLD;

	void init_namco86() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void bankswitch1_w(u8 data);
	void bankswitch1_ext_w(u8 data);
	void bankswitch2_w(u8 data);
	u8 dsw0_r();
	u8 dsw1_r();
	void int_ack1_w(u8 data);
	void int_ack2_w(u8 data);
	template <unsigned Bit> void watchdog_w(u8 data);
	void coin_w(u8 data);
	void led_w(u8 data);
	void cus115_w(offs_t offset, u8 data);
	void tilebank_select_w(offs_t offset, u8 data);
	void backcolor_w(u8 data);

	u32 sprite_pri_cb(u8 attr1, u8 attr2);
	u32 sprite_bank_cb(u32 code, u32 bank);
	void tile_cb_0(u8 layer, u8 &gfxno, u32 &code);
	void tile_cb_1(u8 layer, u8 &gfxno, u32 &code);

	void namcos86_palette(palette_device &palette);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void common_mcu_map(address_map &map) ATTR_COLD;
	void cpu1_map(address_map &map) ATTR_COLD;
	void genpeitd_cpu2_map(address_map &map) ATTR_COLD;
	void genpeitd_mcu_map(address_map &map) ATTR_COLD;
	void hopmappy_cpu2_map(address_map &map) ATTR_COLD;
	void hopmappy_mcu_map(address_map &map) ATTR_COLD;
	void roishtar_cpu2_map(address_map &map) ATTR_COLD;
	void roishtar_mcu_map(address_map &map) ATTR_COLD;
	void rthunder_cpu2_map(address_map &map) ATTR_COLD;
	void rthunder_mcu_map(address_map &map) ATTR_COLD;
	void wndrmomo_cpu2_map(address_map &map) ATTR_COLD;
	void wndrmomo_mcu_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<hd63701v0_cpu_device> m_mcu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<namco_cus30_device> m_cus30;
	required_device<palette_device> m_palette;
	required_device<namcos1_sprite_device> m_spritegen;
	required_device_array<namco_cus4xtmap_device, 2> m_tilegen;
	optional_device<namco_63701x_device> m_63701x;
	optional_region_ptr<u8> m_bankeddata_ptr;
	required_memory_bank m_mainbank;
	optional_memory_bank m_subbank;
	optional_ioport_array<2> m_io_dsw;
	output_finder<2> m_leds;

	u8 m_wdog = 0;
	u32 m_tilebank = 0;
	u16 m_backcolor = 0;
	const u8 *m_tile_address_prom = nullptr;
};

#endif // MAME_NAMCO_NAMCOS86_H
