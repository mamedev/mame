// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Rampart hardware

*************************************************************************/
#ifndef MAME_INCLUDES_RAMPART_H
#define MAME_INCLUDES_RAMPART_H

#pragma once

#include "machine/atarigen.h"
#include "sound/okim6295.h"
#include "sound/ym2413.h"
#include "video/atarimo.h"

class rampart_state : public atarigen_state
{
public:
	rampart_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
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
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(latch_w);
	uint32_t screen_update_rampart(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rampart_bitmap_render(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

private:
	required_device<atari_motion_objects_device> m_mob;
	required_device<okim6295_device> m_oki;
	required_device<ym2413_device> m_ym2413;

	required_shared_ptr<uint16_t> m_bitmap;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_RAMPART_H
