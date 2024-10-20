// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Dan Boris
/*************************************************************************

    rokola hardware

*************************************************************************/
#ifndef MAME_SNK_SNK6502_H
#define MAME_SNK_SNK6502_H

#pragma once

#include "machine/bankdev.h"
#include "machine/timer.h"

#include "emupal.h"
#include "tilemap.h"


class fantasy_sound_device;

class snk6502_state : public driver_device
{
public:
	snk6502_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_charram(*this, "charram")
	{ }

	void satansat(machine_config &config);
	void sasuke(machine_config &config);

	ioport_value sasuke_count_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	DECLARE_VIDEO_START(pballoon);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_charram;

	uint8_t m_sasuke_counter = 0;
	int m_charbank = 0;
	int m_backcolor = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	rgb_t m_palette_val[64]{};
	uint8_t m_irq_mask = 0;

	// common
	void videoram_w(offs_t offset, uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void charram_w(offs_t offset, uint8_t data);

	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void satansat_b002_w(uint8_t data);
	void satansat_backcolor_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(satansat_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(satansat_get_fg_tile_info);

	virtual void machine_start() override ATTR_COLD;
	DECLARE_MACHINE_RESET(sasuke);
	DECLARE_VIDEO_START(satansat);
	void satansat_palette(palette_device &palette);
	DECLARE_VIDEO_START(snk6502);
	void snk6502_palette(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(satansat_interrupt);
	INTERRUPT_GEN_MEMBER(snk6502_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(sasuke_update_counter);

	void sasuke_start_counter();
	void postload();

	void sasuke_map(address_map &map) ATTR_COLD;
	void satansat_map(address_map &map) ATTR_COLD;
};

class vanguard_state : public snk6502_state
{
public:
	vanguard_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk6502_state(mconfig, type, tag),
		m_highmem(*this, "highmem")
	{
	}

	void vanguard(machine_config &config);

protected:
	uint8_t highmem_r(offs_t offset);
	void highmem_w(offs_t offset, uint8_t data);

	required_device<address_map_bank_device> m_highmem;

private:
	void vanguard_map(address_map &map) ATTR_COLD;
	void vanguard_upper_map(address_map &map) ATTR_COLD;
};

class fantasy_state : public vanguard_state
{
public:
	fantasy_state(const machine_config &mconfig, device_type type, const char *tag) :
		vanguard_state(mconfig, type, tag),
		m_sound(*this, "snk6502")
	{
	}

	void fantasy(machine_config &config);
	void nibbler(machine_config &config);
	void pballoon(machine_config &config);

private:
	void fantasy_flipscreen_w(offs_t offset, uint8_t data);

	void fantasy_map(address_map &map) ATTR_COLD;
	void pballoon_map(address_map &map) ATTR_COLD;
	void pballoon_upper_map(address_map &map) ATTR_COLD;

	required_device<fantasy_sound_device> m_sound;
};

#endif // MAME_SNK_SNK6502_H
