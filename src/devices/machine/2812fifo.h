// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**************************************************************************

    2812 32*8 First-In First-Out Memory (AMD, Plessey, and others)
                            ____   ____
                    D0   1 |*   \_/    | 28  D1
                   Vgg   2 |           | 27  D2
                    OR   3 |           | 26  D3
                   /MR   4 |           | 25  /IR
                    PD   5 |           | 24  Vss
                    SD   6 |           | 23  D4
                    Q0   7 |   2812    | 22  D5
                    Q1   8 |           | 21  D6
                    Q2   9 |           | 20  D7
                    Q3  10 |           | 19  FLAG
                    OE  11 |           | 18  PL
                    Q4  12 |           | 17  SL
                    Q5  13 |           | 16  Vdd
                    Q6  14 |___________| 15  Q7

**************************************************************************/
#ifndef MAME_MACHINE_2812FIFO_H
#define MAME_MACHINE_2812FIFO_H

#pragma once

#include <array>


class fifo2812_device : public device_t
{
public:
	// standard constructor
	fifo2812_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	virtual ~fifo2812_device();

	// callbacks
	auto q_cb() { return m_q_cb.bind(); }
	auto ir_cb() { return m_ir_cb.bind(); }
	auto or_cb() { return m_or_cb.bind(); }
	auto flag_cb() { return m_flag_cb.bind(); }

	// control signal interface
	void d_w(u8 data) { m_d = data; }
	void mr_w(int state);
	void pl_w(int state);
	void pd_w(int state);
	void oe_w(int state);
	u8 q_r() const { return m_oe ? m_data[LENGTH - 1] : 0xff; }
	int ir_r() const { return BIT(m_control, 0); }
	int or_r() const { return BIT(m_control, LENGTH - 1); }
	int flag_r();

	// read/write interface
	u8 read();
	void write(u8 data);

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum { LENGTH = 32 };

	// output callbacks
	devcb_write8 m_q_cb;
	devcb_write_line m_ir_cb;
	devcb_write_line m_or_cb;
	devcb_write_line m_flag_cb;

	// registers/state
	u32 m_control;
	std::array<u8, LENGTH> m_data;
	u8 m_count;

	// input line states
	u8 m_d, m_mr, m_pl, m_pd, m_oe;
};

DECLARE_DEVICE_TYPE(FIFO2812, fifo2812_device)

#endif // MAME_MACHINE_2812FIFO_H
