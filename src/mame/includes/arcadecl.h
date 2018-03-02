// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Arcade Classics hardware (prototypes)

*************************************************************************/
#ifndef MAME_INCLUDES_ARCADECL_H
#define MAME_INCLUDES_ARCADECL_H

#pragma once

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "sound/okim6295.h"

class sparkz_state : public atarigen_state
{
public:
	sparkz_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_oki(*this, "oki")
		, m_bitmap(*this, "bitmap")
	{ }

	void sparkz(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(latch_w);
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

private:
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
