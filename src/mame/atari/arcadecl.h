// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Arcade Classics hardware (prototypes)

*************************************************************************/
#ifndef MAME_INCLUDES_ARCADECL_H
#define MAME_INCLUDES_ARCADECL_H

#pragma once

#include "machine/timer.h"
#include "atarimo.h"
#include "sound/okim6295.h"
#include "screen.h"

class sparkz_state : public driver_device
{
public:
	sparkz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_oki(*this, "oki")
		, m_bitmap(*this, "bitmap")
	{ }

	void sparkz(machine_config &config);

protected:
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(uint16_t data);
	void latch_w(uint8_t data);
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<okim6295_device> m_oki;
	required_shared_ptr<uint16_t> m_bitmap;
};


class arcadecl_state : public sparkz_state
{
public:
	arcadecl_state(const machine_config &mconfig, device_type type, const char *tag)
		: sparkz_state(mconfig, type, tag)
		, m_mob(*this, "mob")
	{ }

	void arcadecl(machine_config &config);

protected:
	virtual void video_start() override;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	required_device<atari_motion_objects_device> m_mob;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_ARCADECL_H
