// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca

#ifndef MAME_INCLUDES_TRUCO_H
#define MAME_INCLUDES_TRUCO_H

#pragma once

#include "machine/watchdog.h"
#include "sound/dac.h"
#include "emupal.h"

class truco_state : public driver_device
{
public:
	truco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_videoram(*this, "videoram"),
		m_battery_ram(*this, "battery_ram")
	{ }

	void truco(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<palette_device> m_palette;
	required_device<dac_bit_interface> m_dac;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_battery_ram;

	int m_trigger = 0;

	void porta_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia_ca2_w);
	void portb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia_irqa_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irqb_w);

	void truco_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_TRUCO_H
