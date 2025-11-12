// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
#ifndef MAME_GAELCO_GAELCO2_H
#define MAME_GAELCO_GAELCO2_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "tilemap.h"


class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_spriteram(*this, "spriteram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vregs(*this, "vregs"),
		m_paletteram(*this, "paletteram"),
		m_shareram(*this, "shareram"),
		m_global_spritexoff(0)
	{ }

	void maniacsq_d5002fp(machine_config &config) ATTR_COLD;
	void play2000(machine_config &config) ATTR_COLD;
	void srollnd(machine_config &config) ATTR_COLD;
	void alighunt(machine_config &config) ATTR_COLD;
	void alighunt_d5002fp(machine_config &config) ATTR_COLD;
	void maniacsq(machine_config &config) ATTR_COLD;
	void saltcrdi(machine_config &config) ATTR_COLD;

	void init_alighunt() ATTR_COLD;
	void init_play2000() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void wrally2_latch_w(offs_t offset, u16 data);

	void shareram_w(offs_t offset, u8 data);
	u8 shareram_r(offs_t offset);
	void alighunt_coin_w(u16 data);

	template <unsigned Which> void coin_counter_w(int state);

	void ROM16_split_gfx(const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2);

	template <unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int mask);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_hostmem_map(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	optional_device<ls259_device> m_mainlatch;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_vregs;
	required_shared_ptr<u16> m_paletteram;
	optional_shared_ptr<u16> m_shareram;

	u16 *m_videoram = nullptr;
	tilemap_t *m_pant[2]{};
	bool m_dual_monitor = false;
	int m_global_spritexoff;

private:
	// simulation
	u16 srollnd_share_sim_r(offs_t offset, u16 mem_mask = ~0);
	void srollnd_share_sim_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void alighunt_map(address_map &map) ATTR_COLD;
	void maniacsq_map(address_map &map) ATTR_COLD;
	void play2000_map(address_map &map) ATTR_COLD;
	void saltcrdi_map(address_map &map) ATTR_COLD;
	void srollnd_map(address_map &map) ATTR_COLD;
};

// with dual monitor
class gaelco2_dual_state : public gaelco2_state
{
public:
	gaelco2_dual_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelco2_state(mconfig, type, tag)
	{ }

	void touchgo(machine_config &config) ATTR_COLD;
	void touchgo_d5002fp(machine_config &config) ATTR_COLD;

	void init_touchgo() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	u32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info_dual);
	int get_rowscrollmode_yscroll(bool first_screen);

	u32 dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);

	void touchgo_map(address_map &map) ATTR_COLD;
};

// with protection RAM
class snowboar_state : public gaelco2_state
{
public:
	snowboar_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelco2_state(mconfig, type, tag),
		m_snowboar_protection(*this, "snowboar_prot")
	{ }

	void maniacsqs(machine_config &config) ATTR_COLD;
	void snowboar(machine_config &config) ATTR_COLD;

	void init_snowboara() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u16 snowboar_protection_r();
	void snowboar_protection_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void snowboar_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u16> m_snowboar_protection;

	u32 m_snowboard_latch = 0U;
};

// with gun IO
class bang_state : public gaelco2_state
{
public:
	bang_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_light_x(*this, "LIGHT%u_X", 0U)
		, m_light_y(*this, "LIGHT%u_Y", 0U)
	{}

	void bang(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	template <unsigned Which> u16 gun_x();
	template <unsigned Which> u16 gun_y();
	void bang_clr_gun_int_w(u16 data);
	TIMER_DEVICE_CALLBACK_MEMBER(bang_irq);

	void bang_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_light_x;
	required_ioport_array<2> m_light_y;

	bool m_clr_gun_int = false;
};

// with ADC
class wrally2_state : public gaelco2_dual_state
{
public:
	wrally2_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_dual_state(mconfig, type, tag)
		, m_analog(*this, "ANALOG%u", 0U)
	{}

	void wrally2(machine_config &config) ATTR_COLD;

	void init_wrally2() ATTR_COLD;

	template <int N> int wrally2_analog_bit_r();

private:
	void wrally2_adc_clk(int state);
	void wrally2_adc_cs(int state);

	void wrally2_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_analog;

	u8 m_analog_ports[2]{};
};

#endif // MAME_GAELCO_GAELCO2_H
