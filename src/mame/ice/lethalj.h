// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    The Game Room Lethal Justice hardware

**************************************************************************/
#ifndef MAME_ICE_LETHALJ_H
#define MAME_ICE_LETHALJ_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "machine/ticket.h"
#include "screen.h"


class lethalj_state : public driver_device
{
public:
	lethalj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_blitter_base(*this, "gfx"),
		m_paddle(*this, "PADDLE"),
		m_light0_x(*this, "LIGHT0_X"),
		m_light0_y(*this, "LIGHT0_Y"),
		m_light1_x(*this, "LIGHT1_X"),
		m_light1_y(*this, "LIGHT1_Y"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void lethalj(machine_config &config);
	void gameroom(machine_config &config);
	void franticf(machine_config &config);

	void init_cfarm();
	void init_ripribit();
	void init_cclownz();

	ioport_value cclownz_paddle();

private:
	void ripribit_control_w(uint16_t data);
	void cfarm_control_w(uint16_t data);
	void cclownz_control_w(uint16_t data);
	uint16_t lethalj_gun_r(offs_t offset);
	void blitter_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void do_blit();
	inline void get_crosshair_xy(int player, int *x, int *y);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	void lethalj_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(gen_ext1_int);

	required_device<tms34010_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ticket_dispenser_device> m_ticket;

	required_region_ptr<uint16_t> m_blitter_base;

	optional_ioport m_paddle;
	optional_ioport m_light0_x;
	optional_ioport m_light0_y;
	optional_ioport m_light1_x;
	optional_ioport m_light1_y;
	output_finder<3> m_lamps;

	emu_timer *m_gen_ext1_int_timer = nullptr;
	uint16_t m_blitter_data[8]{};
	std::unique_ptr<uint16_t[]> m_screenram;
	uint8_t m_vispage = 0;
	int m_blitter_rows = 0;
	uint16_t m_gunx = 0;
	uint16_t m_guny = 0;
	uint8_t m_blank_palette = 0;
};

#endif // MAME_ICE_LETHALJ_H
