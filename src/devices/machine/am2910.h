// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    am2910.h
    AMD Am2910 Microprogram Controller emulation

***************************************************************************/

#ifndef MAME_MACHINE_AM2910_AM2910_H
#define MAME_MACHINE_AM2910_AM2910_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> am2910_device

class am2910_device : public device_t
{
public:
	// construction/destruction
	am2910_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(cc_w);    // !CC
	DECLARE_WRITE_LINE_MEMBER(ccen_w);  // !CCEN
	DECLARE_WRITE_LINE_MEMBER(ci_w);    // CI
	DECLARE_WRITE_LINE_MEMBER(rld_w);   // !RLD
	DECLARE_WRITE_LINE_MEMBER(cp_w);    // CP
	void d_w(uint16_t data);
	void i_w(uint8_t data);

	auto y() { return m_y.bind(); }
	auto pl() { return m_pl.bind(); }
	auto full() { return m_full.bind(); }
	auto map() { return m_map.bind(); }
	auto vect() { return m_vect.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static constexpr device_timer_id TIMER_CLOCK = 0;

	emu_timer *m_execute_timer;

	void execute();
	bool test_pass();
	void push(uint16_t value);
	void pop();
	void update_source();

	// internal state
	uint16_t m_pc;
	uint16_t m_r;
	uint8_t m_sp;
	uint16_t m_stack[5];

	// inputs
	int m_cc;
	int m_ccen;
	int m_ci;
	int m_rld;
	int m_cp;
	uint16_t m_d;
	uint8_t m_i;

	// outputs
	devcb_write16 m_y;
	devcb_write_line m_full;
	devcb_write_line m_pl;
	devcb_write_line m_map;
	devcb_write_line m_vect;
};

// device type definition
DECLARE_DEVICE_TYPE(AM2910, am2910_device)

#endif // MAME_MACHINE_AM2910_AM2910_H
