// license:BSD-3-Clause
// copyright-holders:hap
/*

    Milton Bradley Grand Master motorized self-moving chessboard

*/

#ifndef MAME_MACHINE_GMBOARD_H
#define MAME_MACHINE_GMBOARD_H

#pragma once

#include "machine/sensorboard.h"

class gmboard_device : public device_t
{
public:
	gmboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	template <std::size_t N> auto quad_cb() { return m_quad_cb[N].bind(); } // x/y motor quadrature encoder state
	gmboard_device &set_size(u16 width, u16 height, u16 square) { m_width = width; m_height = height; m_square = square; return *this; }
	gmboard_device &set_offsets(u16 x_offset, u16 y_offset) { m_x_offset = x_offset; m_y_offset = y_offset; return *this; }
	gmboard_device &set_speed(attotime speed) { m_speed = speed; return *this; }

	sensorboard_device &get() { return *m_board; }

	// external read/write handlers
	int magnet_r() { return (started() && m_piece_hand) ? 1 : 0; }
	void magnet_w(int state);
	void motor_w(offs_t offset, u8 data);
	u8 quad_r(offs_t offset) { return m_motor_quad[offset & 1]; }

	u16 read_file(u8 x, bool reverse = false) { return m_board->read_file(x, reverse); }
	u16 read_rank(u8 y, bool reverse = false) { return m_board->read_rank(y, reverse); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<sensorboard_device> m_board;
	output_finder<> m_piece_hand;
	output_finder<5> m_out_motor;
	output_finder<2> m_out_pos;

	u8 m_magnet;
	u8 m_pieces_map[0x40][0x40];

	u8 m_motor_dir[2];
	u32 m_motor_max[2];
	u32 m_motor_pos[2];
	u8 m_motor_quad[2];
	s32 m_motor_drift[2]; // diagnostics

	attotime m_motor_period;
	attotime m_motor_remain[2];
	emu_timer *m_motor_timer[2];

	u16 m_width;          // motor range
	u16 m_height;         // "
	u16 m_square;         // number of quarter rotation steps per square
	u16 m_x_offset;       // unscaled offset relative to the bottom-left corner
	u16 m_y_offset;       // "
	attotime m_speed;     // time per square at full speed

	devcb_write8::array<2> m_quad_cb;

	void init_board(u8 data);
	void clear_board(u8 data);
	void init_motors();

	void get_scaled_pos(double *x, double *y);
	void output_magnet_pos();
	void realign_magnet_pos();

	TIMER_CALLBACK_MEMBER(motor_count);
};


DECLARE_DEVICE_TYPE(MB_GMBOARD, gmboard_device)

#endif // MAME_MACHINE_GMBOARD_H
