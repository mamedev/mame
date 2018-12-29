// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Grull Osgo
#ifndef MAME_INCLUDES_AMPOKER2_H
#define MAME_INCLUDES_AMPOKER2_H

#pragma once

#include "machine/watchdog.h"
#include "emupal.h"

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
	virtual void video_start() override;
	virtual void machine_start() override;

private:
	DECLARE_WRITE8_MEMBER(port30_w);
	DECLARE_WRITE8_MEMBER(port31_w);
	DECLARE_WRITE8_MEMBER(port32_w);
	DECLARE_WRITE8_MEMBER(port33_w);
	DECLARE_WRITE8_MEMBER(port34_w);
	DECLARE_WRITE8_MEMBER(port35_w);
	DECLARE_WRITE8_MEMBER(port36_w);
	DECLARE_WRITE8_MEMBER(watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(s2k_get_bg_tile_info);
	void ampoker2_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(sigma2k);

	void io_map(address_map &map);
	void program_map(address_map &map);

	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_bg_tilemap;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<10> m_lamps;
};

#endif // MAME_INCLUDES_AMPOKER2_H
