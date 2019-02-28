// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder
#ifndef MAME_INCLUDES_LVCARDS_H
#define MAME_INCLUDES_LVCARDS_H

#pragma once

#include "emupal.h"

class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void lvcards(machine_config &config);

protected:
	virtual void video_start() override;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);

	required_device<cpu_device> m_maincpu;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void lvcards_palette(palette_device &palette) const;
	uint32_t screen_update_lvcards(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<gfxdecode_device> m_gfxdecode;
	void lvcards_io_map(address_map &map);
	void lvcards_map(address_map &map);
};


class lvpoker_state : public lvcards_state
{
public:
	using lvcards_state::lvcards_state;

	void lvpoker(machine_config &config);
	void ponttehk(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_WRITE8_MEMBER(control_port_2_w);
	DECLARE_WRITE8_MEMBER(control_port_2a_w);
	DECLARE_READ8_MEMBER(payout_r);

	void lvpoker_map(address_map &map);
	void ponttehk_map(address_map &map);

	uint8_t m_payout;
	uint8_t m_pulse;
	uint8_t m_result;
};

#endif // MAME_INCLUDES_LVCARDS_H
