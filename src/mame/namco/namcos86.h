// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_NAMCOS86_H
#define MAME_NAMCO_NAMCOS86_H

#pragma once

#include "namco_cus4xtmap.h"

#include "cpu/m6800/m6801.h"
#include "machine/watchdog.h"
#include "sound/n63701x.h"
#include "sound/namco.h"

#include "emupal.h"
#include "tilemap.h"

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
		, m_tilegen(*this, "tilegen_%u", 0U)
		, m_63701x(*this, "namco2")
		, m_spriteram(*this, "spriteram")
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
	void bankswitch1_w(uint8_t data);
	void bankswitch1_ext_w(uint8_t data);
	void bankswitch2_w(uint8_t data);
	uint8_t dsw0_r();
	uint8_t dsw1_r();
	void int_ack1_w(uint8_t data);
	void int_ack2_w(uint8_t data);
	template <unsigned Bit> void watchdog_w(uint8_t data);
	void coin_w(uint8_t data);
	void led_w(uint8_t data);
	void cus115_w(offs_t offset, uint8_t data);
	void tilebank_select_w(offs_t offset, uint8_t data);
	void backcolor_w(uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);

	void tile_cb_0(u8 layer, u8 &gfxno, u32 &code);
	void tile_cb_1(u8 layer, u8 &gfxno, u32 &code);

	void namcos86_palette(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<namco_cus4xtmap_device, 2> m_tilegen;
	optional_device<namco_63701x_device> m_63701x;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_region_ptr<uint8_t> m_bankeddata_ptr;
	required_memory_bank m_mainbank;
	optional_memory_bank m_subbank;
	optional_ioport_array<2> m_io_dsw;
	output_finder<2> m_leds;

	uint8_t m_wdog = 0;
	uint32_t m_tilebank = 0;
	uint16_t m_backcolor = 0;
	const uint8_t *m_tile_address_prom = nullptr;
	bool m_copy_sprites = false;
};

#endif // MAME_NAMCO_NAMCOS86_H
