// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * machine/tpi6525.h
 *
 * mos tri port interface 6525
 * mos triple interface adapter 6523
 *
 * peter.trauner@jk.uni-linz.ac.at
 *
 * used in commodore b series
 * used in commodore c1551 floppy disk drive
 *
 * tia6523 is a tpi6525 without control register!?
 *
 * tia6523
 *   only some lines of port b and c are in the pinout!
 *
 * connector to floppy c1551 (delivered with c1551 as c16 expansion)
 *   port a for data read/write
 *   port b
 *   0 status 0
 *   1 status 1
 *   port c
 *   6 dav output edge data on port a available
 *   7 ack input edge ready for next datum
 *
 ****************************************************************************/

#ifndef MAME_MACHINE_6525TPI_H
#define MAME_MACHINE_6525TPI_H

#pragma once


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class tpi6525_device : public device_t
{
public:
	tpi6525_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_irq_cb() { return m_out_irq_cb.bind(); }
	auto in_pa_cb() { return m_in_pa_cb.bind(); }
	auto out_pa_cb() { return m_out_pa_cb.bind(); }
	auto in_pb_cb() { return m_in_pb_cb.bind(); }
	auto out_pb_cb() { return m_out_pb_cb.bind(); }
	auto in_pc_cb() { return m_in_pc_cb.bind(); }
	auto out_pc_cb() { return m_out_pc_cb.bind(); }
	auto out_ca_cb() { return m_out_ca_cb.bind(); }
	auto out_cb_cb() { return m_out_cb_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void i0_w(int state);
	void i1_w(int state);
	void i2_w(int state);
	void i3_w(int state);
	void i4_w(int state);

	uint8_t pa_r();
	uint8_t pb_r();
	uint8_t pc_r();
	void pa_w(uint8_t data);
	void pb_w(uint8_t data);
	void pc_w(uint8_t data);

	void pb0_w(int state) { port_line_w(m_in_b, 0, state); }
	void pb1_w(int state) { port_line_w(m_in_b, 1, state); }
	void pb2_w(int state) { port_line_w(m_in_b, 2, state); }
	void pb3_w(int state) { port_line_w(m_in_b, 3, state); }
	void pb4_w(int state) { port_line_w(m_in_b, 4, state); }
	void pb5_w(int state) { port_line_w(m_in_b, 5, state); }
	void pb6_w(int state) { port_line_w(m_in_b, 6, state); }
	void pb7_w(int state) { port_line_w(m_in_b, 7, state); }

	uint8_t get_ddr_a();
	uint8_t get_ddr_b();
	uint8_t get_ddr_c();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	devcb_write_line    m_out_irq_cb;

	devcb_read8         m_in_pa_cb;
	devcb_write8        m_out_pa_cb;

	devcb_read8         m_in_pb_cb;
	devcb_write8        m_out_pb_cb;

	devcb_read8         m_in_pc_cb;
	devcb_write8        m_out_pc_cb;

	devcb_write_line    m_out_ca_cb;
	devcb_write_line    m_out_cb_cb;

	uint8_t m_port_a, m_ddr_a, m_in_a;
	uint8_t m_port_b, m_ddr_b, m_in_b;
	uint8_t m_port_c, m_ddr_c, m_in_c;

	uint8_t m_ca_level, m_cb_level, m_interrupt_level;

	uint8_t m_cr;
	uint8_t m_air;

	uint8_t m_irq_level[5];

	void set_interrupt();
	void clear_interrupt();

	// helper function to write a single line
	static void port_line_w(uint8_t &port, int line, int state);
};

DECLARE_DEVICE_TYPE(TPI6525, tpi6525_device)

#endif // MAME_MACHINE_6525TPI_H
