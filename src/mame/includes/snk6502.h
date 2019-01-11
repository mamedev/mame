// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Dan Boris
/*************************************************************************

    rokola hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SNK6502_H
#define MAME_INCLUDES_SNK6502_H

#pragma once

#include "machine/timer.h"
#include "emupal.h"

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
	void vanguard(machine_config &config);
	void sasuke(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(sasuke_count_r);
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

	uint8_t m_sasuke_counter;
	int m_charbank;
	int m_backcolor;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	rgb_t m_palette_val[64];
	uint8_t m_irq_mask;

	// common
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(charram_w);

	DECLARE_WRITE8_MEMBER(scrollx_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(satansat_b002_w);
	DECLARE_WRITE8_MEMBER(satansat_backcolor_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(satansat_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(satansat_get_fg_tile_info);

	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(sasuke);
	DECLARE_VIDEO_START(satansat);
	void satansat_palette(palette_device &palette);
	DECLARE_VIDEO_START(snk6502);
	void snk6502_palette(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(satansat_interrupt);
	INTERRUPT_GEN_MEMBER(snk6502_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(sasuke_update_counter);

	void sasuke_start_counter();
	void postload();

	void sasuke_map(address_map &map);
	void satansat_map(address_map &map);
	void vanguard_map(address_map &map);
};

class fantasy_state : public snk6502_state
{
public:
	fantasy_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk6502_state(mconfig, type, tag),
		m_sound(*this, "snk6502")
	{
	}

	void fantasy(machine_config &config);
	void nibbler(machine_config &config);
	void pballoon(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(fantasy_flipscreen_w);

	void fantasy_map(address_map &map);
	void pballoon_map(address_map &map);

	required_device<fantasy_sound_device> m_sound;
};

#endif // MAME_INCLUDES_SNK6502_H
