// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Grull Osgo
#ifndef MAME_MISC_AMPOKER2_H
#define MAME_MISC_AMPOKER2_H

#pragma once

#include "machine/watchdog.h"
#include "emupal.h"
#include "tilemap.h"

class ampoker2_state : public driver_device
{
public:
	ampoker2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void sigma2k(machine_config &config);
	void ampoker2(machine_config &config);

	void init_rabbitpk();

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void port30_w(uint8_t data);
	void port31_w(uint8_t data);
	void port32_w(uint8_t data);
	void port33_w(uint8_t data);
	void port34_w(uint8_t data);
	void port35_w(uint8_t data);
	void port36_w(uint8_t data);
	void watchdog_reset_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(s2k_get_bg_tile_info);
	void ampoker2_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(sigma2k);

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_bg_tilemap = nullptr;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<10> m_lamps;
};

#endif // MAME_MISC_AMPOKER2_H
