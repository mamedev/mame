// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Rampart hardware

*************************************************************************/
#ifndef MAME_INCLUDES_RAMPART_H
#define MAME_INCLUDES_RAMPART_H

#pragma once

#include "machine/slapstic.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ym2413.h"
#include "video/atarimo.h"
#include "screen.h"

class rampart_state : public driver_device
{
public:
	rampart_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slapstic(*this, "slapstic"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_mob(*this, "mob"),
		m_oki(*this, "oki"),
		m_ym2413(*this, "ymsnd"),
		m_bitmap(*this, "bitmap")
	{ }

	void init_rampart();
	void rampart(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(uint16_t data);
	void latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update_rampart(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rampart_bitmap_render(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<atari_slapstic_device> m_slapstic;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_motion_objects_device> m_mob;
	required_device<okim6295_device> m_oki;
	required_device<ym2413_device> m_ym2413;

	required_shared_ptr<uint16_t> m_bitmap;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_RAMPART_H
