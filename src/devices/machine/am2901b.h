// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    am2901b.h
    AMD Am2901B / Am2901C
    Four-Bit Bipolar Microprocessor Slice

***************************************************************************/

#ifndef MAME_MACHINE_AM2901B_H
#define MAME_MACHINE_AM2901B_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> am2901b_device

class am2901b_device : public device_t
{
public:
	// construction/destruction
	am2901b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto y() { return m_y.bind(); }
	auto g() { return m_g.bind(); }
	auto p() { return m_p.bind(); }
	auto q0() { return m_q0.bind(); }
	auto q3() { return m_q3.bind(); }
	auto ram0() { return m_ram0.bind(); }
	auto ram3() { return m_ram3.bind(); }
	auto ovr() { return m_ovr.bind(); }
	auto f0() { return m_f0.bind(); }
	auto f3() { return m_f3.bind(); }

	void a_w(uint8_t data);
	void b_w(uint8_t data);
	void d_w(uint8_t data);
	void i_w(uint16_t data);

	void q0_w(int state);
	void q3_w(int state);
	void ram0_w(int state);
	void ram3_w(int state);
	void ci_w(int state);
	void cp_w(int state);

private:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void execute();
	void disassemble();

	uint8_t m_a[16];
	uint8_t m_d;
	uint8_t m_q;
	uint16_t m_i;

	uint8_t m_a_addr;
	uint8_t m_b_addr;

	uint8_t m_a_latch;
	uint8_t m_b_latch;

	bool m_q0_in;
	bool m_q3_in;
	bool m_ram0_in;
	bool m_ram3_in;
	bool m_ci;
	bool m_cp;

	uint8_t m_y_out;
	bool m_g_out;
	bool m_p_out;
	bool m_q0_out;
	bool m_q3_out;
	bool m_ram0_out;
	bool m_ram3_out;
	bool m_ovr_out;
	bool m_f0_out;
	bool m_f3_out;
	bool m_co_out;

	devcb_write8 m_y;
	devcb_write_line m_g;
	devcb_write_line m_p;
	devcb_write_line m_q0;
	devcb_write_line m_q3;
	devcb_write_line m_ram0;
	devcb_write_line m_ram3;
	devcb_write_line m_ovr;
	devcb_write_line m_f0;
	devcb_write_line m_f3;
	devcb_write_line m_co;
};

// device type definition
DECLARE_DEVICE_TYPE(AM2901B, am2901b_device)

#endif // MAME_MACHINE_AM2901B_H
