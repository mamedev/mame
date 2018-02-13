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

	lethalj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_paddle(*this, "PADDLE"),
		m_light0_x(*this, "LIGHT0_X"),
		m_light0_y(*this, "LIGHT0_Y"),
		m_light1_x(*this, "LIGHT1_X"),
		m_light1_y(*this, "LIGHT1_Y")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ticket_dispenser_device> m_ticket;
	optional_ioport m_paddle;
	optional_ioport m_light0_x;
	optional_ioport m_light0_y;
	optional_ioport m_light1_x;
	optional_ioport m_light1_y;

	emu_timer *m_gen_ext1_int_timer;
	uint16_t m_blitter_data[8];
	std::unique_ptr<uint16_t[]> m_screenram;
	uint8_t m_vispage;
	uint16_t *m_blitter_base;
	int m_blitter_rows;
	uint16_t m_gunx;
	uint16_t m_guny;
	uint8_t m_blank_palette;
	DECLARE_WRITE16_MEMBER(ripribit_control_w);
	DECLARE_WRITE16_MEMBER(cfarm_control_w);
	DECLARE_WRITE16_MEMBER(cclownz_control_w);
	DECLARE_READ16_MEMBER(lethalj_gun_r);
	DECLARE_WRITE16_MEMBER(lethalj_blitter_w);
	void do_blit();
	DECLARE_CUSTOM_INPUT_MEMBER(cclownz_paddle);
	DECLARE_DRIVER_INIT(cfarm);
	DECLARE_DRIVER_INIT(ripribit);
	DECLARE_DRIVER_INIT(cclownz);
	virtual void video_start() override;
	inline void get_crosshair_xy(int player, int *x, int *y);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	void lethalj(machine_config &config);
	void gameroom(machine_config &config);
	void lethalj_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_LETHALJ_H
