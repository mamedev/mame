// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Taito O system

*************************************************************************/
#ifndef MAME_INCLUDES_TAITO_O_H
#define MAME_INCLUDES_TAITO_O_H

#pragma once

#include "machine/timer.h"
#include "machine/watchdog.h"
#include "video/tc0080vco.h"
#include "emupal.h"

class taitoo_state : public driver_device
{
public:
	taitoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_tc0080vco(*this, "tc0080vco"),
		m_palette(*this, "palette"),
		m_io_in(*this, "IN%u", 0U)
	{ }

	void parentj(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<tc0080vco_device> m_tc0080vco;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_io_in;

	void io_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 io_r(offs_t offset, u16 mem_mask = ~0);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(parentj_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void parentj_map(address_map &map);
};

#endif // MAME_INCLUDES_TAITO_O_H
