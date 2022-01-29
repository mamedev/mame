// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pocketc.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_POCKETC_H
#define MAME_INCLUDES_POCKETC_H

#pragma once

#include "cpu/sc61860/sc61860.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"

class pocketc_state : public driver_device
{
public:
	pocketc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_cpu_nvram(*this, "cpu_nvram")
		, m_ram_nvram(*this, "ram_nvram")
		, m_dsw0(*this, "DSW0")
		, m_extra(*this, "EXTRA")
		, m_power_timer(nullptr)
	{ }

	void pocketc_base(machine_config &config);

protected:
	static const device_timer_id TIMER_POWER_UP = 0;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	void pocketc_palette(palette_device &palette) const;

	void pocketc_draw_special(bitmap_ind16 &bitmap,int x, int y, const char* const *fig, int color);

	void out_a_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER(brk_r);

	required_device<sc61860_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<nvram_device> m_cpu_nvram;
	required_device<nvram_device> m_ram_nvram;
	required_ioport m_dsw0;
	required_ioport m_extra;

	uint8_t m_outa;
	uint8_t m_outb;
	int m_power;
	emu_timer *m_power_timer;

	static const int colortable[8][2];
	static const rgb_t indirect_palette[6];
};

#endif // MAME_INCLUDES_POCKETC_H
