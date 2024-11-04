// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
#ifndef MAME_FUNWORLD_SNOOKR10_H
#define MAME_FUNWORLD_SNOOKR10_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class snookr10_state : public driver_device
{
public:
	snookr10_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void apple10(machine_config &config);
	void snookr10(machine_config &config);
	void crystalc(machine_config &config);
	void tenballs(machine_config &config);

private:
	uint8_t dsw_port_1_r();
	uint8_t port2000_8_r();
	void output_port_0_w(uint8_t data);
	void output_port_1_w(uint8_t data);
	void snookr10_videoram_w(offs_t offset, uint8_t data);
	void snookr10_colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(apple10_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(crystalc_get_bg_tile_info);
	void snookr10_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(apple10);
	DECLARE_VIDEO_START(crystalc);
	void apple10_palette(palette_device &palette) const;
	void crystalc_palette(palette_device &palette) const;
	uint32_t screen_update_snookr10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void crystalc_map(address_map &map) ATTR_COLD;
	void snookr10_map(address_map &map) ATTR_COLD;
	void tenballs_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

	int m_outportl = 0;
	int m_outporth = 0;
	int m_bit0 = 0;
	int m_bit1 = 0;
	int m_bit2 = 0;
	int m_bit3 = 0;
	int m_bit4 = 0;
	int m_bit5 = 0;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<7> m_lamps;
};

#endif // MAME_FUNWORLD_SNOOKR10_H
