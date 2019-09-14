// license:BSD-3-Clause
// copyright-holders:Richard Davies
#ifndef MAME_INCLUDES_PHOENIX_H
#define MAME_INCLUDES_PHOENIX_H

#pragma once

#include "audio/pleiads.h"
#include "emupal.h"
#include "tilemap.h"

class phoenix_state : public driver_device
{
public:
	phoenix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pleiads_custom(*this, "pleiads_custom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_fg_tilemap(nullptr)
		, m_bg_tilemap(nullptr)
	{
	}

	DECLARE_WRITE8_MEMBER(phoenix_videoram_w);
	DECLARE_WRITE8_MEMBER(phoenix_videoreg_w);
	DECLARE_WRITE8_MEMBER(pleiads_videoreg_w);
	DECLARE_WRITE8_MEMBER(phoenix_scroll_w);
	DECLARE_READ8_MEMBER(survival_input_port_0_r);
	DECLARE_CUSTOM_INPUT_MEMBER(player_input_r);
	DECLARE_READ_LINE_MEMBER(pleiads_protection_r);
	void init_oneprom();
	void init_coindsw();
	void init_oneprom_coindsw();
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_RESET(phoenix);
	DECLARE_VIDEO_START(phoenix);
	void phoenix_palette(palette_device &palette) const;
	void survival_palette(palette_device &palette) const;
	void pleiads_palette(palette_device &palette) const;
	uint32_t screen_update_phoenix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(survival_protection_r);
	DECLARE_READ_LINE_MEMBER(survival_sid_callback);

	void condor(machine_config &config);
	void phoenix(machine_config &config);
	void survival(machine_config &config);
	void pleiads(machine_config &config);
	void phoenix_memory_map(address_map &map);
	void pleiads_memory_map(address_map &map);
	void survival_memory_map(address_map &map);
protected:
	required_device<cpu_device>             m_maincpu;
	optional_device<pleiads_sound_device>   m_pleiads_custom;
	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<palette_device>         m_palette;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	std::unique_ptr<uint8_t[]> m_videoram_pg[2];
	uint8_t m_videoram_pg_index;
	uint8_t m_palette_bank;
	uint8_t m_cocktail_mode;
	uint8_t m_pleiads_protection_question;
	uint8_t m_survival_protection_value;
	int m_survival_sid_value;
	uint8_t m_survival_input_latches[2];
	uint8_t m_survival_input_readc;
};


/*----------- video timing  -----------*/

#define MASTER_CLOCK            XTAL(11'000'000)

#define PIXEL_CLOCK             (MASTER_CLOCK/2)
#define CPU_CLOCK               (PIXEL_CLOCK)
#define HTOTAL                  (512-160)
#define HBSTART                 (256)
#define HBEND                   (0)
#define VTOTAL                  (256)
#define VBSTART                 (208)
#define VBEND                   (0)

#endif // MAME_INCLUDES_PHOENIX_H
