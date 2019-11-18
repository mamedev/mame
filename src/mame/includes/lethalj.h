// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    The Game Room Lethal Justice hardware

**************************************************************************/
#ifndef MAME_INCLUDES_LETHALJ_H
#define MAME_INCLUDES_LETHALJ_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "machine/ticket.h"
#include "screen.h"


class lethalj_state : public driver_device
{
public:
	enum
	{
		TIMER_GEN_EXT1_INT
	};

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

	void init_cfarm();
	void init_ripribit();
	void init_cclownz();

	DECLARE_CUSTOM_INPUT_MEMBER(cclownz_paddle);

private:
	DECLARE_WRITE16_MEMBER(ripribit_control_w);
	DECLARE_WRITE16_MEMBER(cfarm_control_w);
	DECLARE_WRITE16_MEMBER(cclownz_control_w);
	DECLARE_READ16_MEMBER(lethalj_gun_r);
	DECLARE_WRITE16_MEMBER(blitter_w);
	void do_blit();
	inline void get_crosshair_xy(int player, int *x, int *y);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	void lethalj_map(address_map &map);

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void video_start() override;

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

	emu_timer *m_gen_ext1_int_timer;
	uint16_t m_blitter_data[8];
	std::unique_ptr<uint16_t[]> m_screenram;
	uint8_t m_vispage;
	int m_blitter_rows;
	uint16_t m_gunx;
	uint16_t m_guny;
	uint8_t m_blank_palette;
};

#endif // MAME_INCLUDES_LETHALJ_H
