// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Nathan Woods
/**********************************************************************

    Motorola 6821 PIA interface and emulation

    Notes:
        * port_b_z_mask() gives the caller the bitmask that shows
          which bits are high-impedance when reading port B, and thus
          neither 0 or 1. cb2_output_z() returns the same info
          for the CB2 pin.
        * The 'alt' interface functions are used when the A0 and A1
          address bits are swapped.
        * All 'int' data or return values are bool, and should be
          converted to bool at some point.

**********************************************************************/

#ifndef MAME_DEVICES_MACHINE_6821PIA_H
#define MAME_DEVICES_MACHINE_6821PIA_H

#pragma once




/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> pia6821_device

class pia6821_device :  public device_t
{
public:
	// construction/destruction
	pia6821_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// TODO: REMOVE THESE
	auto readpa_handler() { return m_in_a_handler.bind(); }
	auto readpb_handler() { return m_in_b_handler.bind(); }
	auto readca1_handler() { return m_in_ca1_handler.bind(); }
	auto readca2_handler() { return m_in_ca2_handler.bind(); }
	auto readcb1_handler() { return m_in_cb1_handler.bind(); }

	auto writepa_handler() { return m_out_a_handler.bind(); }
	auto writepb_handler() { return m_out_b_handler.bind(); }
	auto tspb_handler() { return m_ts_b_handler.bind(); }

	auto ca2_handler() { return m_ca2_handler.bind(); }
	auto cb2_handler() { return m_cb2_handler.bind(); }
	auto irqa_handler() { return m_irqa_handler.bind(); }
	auto irqb_handler() { return m_irqb_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t read_alt(offs_t offset) { return read(((offset << 1) & 0x02) | ((offset >> 1) & 0x01)); }
	void write_alt(offs_t offset, uint8_t data) { write(((offset << 1) & 0x02) | ((offset >> 1) & 0x01), data); }

	uint8_t port_b_z_mask() const { return ~m_ddr_b; } // see notes

	void porta_w(uint8_t data);
	void write_porta_line(int line, bool state);
	void set_a_input(uint8_t data);
	uint8_t a_output();
	void set_port_a_input_overrides_output_mask(uint8_t mask) { m_a_input_overrides_output_mask = mask; }

	void pa0_w(int state) { write_porta_line(0, state); }
	void pa1_w(int state) { write_porta_line(1, state); }
	void pa2_w(int state) { write_porta_line(2, state); }
	void pa3_w(int state) { write_porta_line(3, state); }
	void pa4_w(int state) { write_porta_line(4, state); }
	void pa5_w(int state) { write_porta_line(5, state); }
	void pa6_w(int state) { write_porta_line(6, state); }
	void pa7_w(int state) { write_porta_line(7, state); }

	void ca1_w(int state);

	void ca2_w(int state);
	bool ca2_output();
	bool ca2_output_z();

	void portb_w(uint8_t data);
	void write_portb_line(int line, bool state);
	uint8_t b_output();

	void pb0_w(int state) { write_portb_line(0, state); }
	void pb1_w(int state) { write_portb_line(1, state); }
	void pb2_w(int state) { write_portb_line(2, state); }
	void pb3_w(int state) { write_portb_line(3, state); }
	void pb4_w(int state) { write_portb_line(4, state); }
	void pb5_w(int state) { write_portb_line(5, state); }
	void pb6_w(int state) { write_portb_line(6, state); }
	void pb7_w(int state) { write_portb_line(7, state); }

	void cb1_w(int state);

	void cb2_w(int state);
	bool cb2_output();
	bool cb2_output_z();

	int irq_a_state() const { return m_irq_a_state; }
	int irq_b_state() const { return m_irq_b_state; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

	void update_interrupts();

	uint8_t get_in_a_value();
	uint8_t get_in_b_value();

	uint8_t get_out_a_value();
	uint8_t get_out_b_value();

	void set_out_ca2(int data);
	void set_out_cb2(int data);

	uint8_t port_a_r();
	uint8_t ddr_a_r();
	uint8_t control_a_r();

	uint8_t port_b_r();
	uint8_t ddr_b_r();
	uint8_t control_b_r();

	void send_to_out_a_func(const char* message);
	void send_to_out_b_func(const char* message);

	void port_a_w(uint8_t data);
	void ddr_a_w(uint8_t data);

	void port_b_w(uint8_t data);
	void ddr_b_w(uint8_t data);

	void control_a_w(uint8_t data);
	void control_b_w(uint8_t data);

	static bool irq1_enabled(uint8_t c);
	static bool c1_low_to_high(uint8_t c);
	static bool c1_high_to_low(uint8_t c);
	static bool output_selected(uint8_t c);
	static bool irq2_enabled(uint8_t c);
	static bool strobe_e_reset(uint8_t c);
	static bool strobe_c1_reset(uint8_t c);
	static bool c2_set(uint8_t c);
	static bool c2_low_to_high(uint8_t c);
	static bool c2_high_to_low(uint8_t c);
	static bool c2_set_mode(uint8_t c);
	static bool c2_strobe_mode(uint8_t c);
	static bool c2_output(uint8_t c);
	static bool c2_input(uint8_t c);

	devcb_read8 m_in_a_handler;
	devcb_read8 m_in_b_handler;
	devcb_read_line m_in_ca1_handler;
	devcb_read_line m_in_cb1_handler;
	devcb_read_line m_in_ca2_handler;
	devcb_write8 m_out_a_handler;
	devcb_write8 m_out_b_handler;
	devcb_read8 m_ts_b_handler;
	devcb_write_line m_ca2_handler;
	devcb_write_line m_cb2_handler;
	devcb_write_line m_irqa_handler;
	devcb_write_line m_irqb_handler;

	uint8_t m_in_a;
	uint8_t m_in_ca1;
	uint8_t m_in_ca2;
	uint8_t m_out_a;
	uint8_t m_a_input_overrides_output_mask;
	uint8_t m_out_ca2;
	uint8_t m_ddr_a;
	uint8_t m_ctl_a;
	bool m_irq_a1;
	bool m_irq_a2;
	uint8_t m_irq_a_state;

	uint8_t m_in_b;
	uint8_t m_in_cb1;
	uint8_t m_in_cb2;
	uint8_t m_out_b;
	uint8_t m_out_cb2;
	uint8_t m_last_out_cb2_z;
	uint8_t m_ddr_b;
	uint8_t m_ctl_b;
	bool m_irq_b1;
	bool m_irq_b2;
	uint8_t m_irq_b_state;

	// variables that indicate if access a line externally -
	// used to for logging purposes ONLY
	bool m_in_a_pushed;
	bool m_out_a_needs_pulled;
	bool m_in_ca1_pushed;
	bool m_in_ca2_pushed;
	bool m_out_ca2_needs_pulled;
	bool m_in_b_pushed;
	bool m_out_b_needs_pulled;
	bool m_in_cb1_pushed;
	bool m_in_cb2_pushed;
	bool m_out_cb2_needs_pulled;
	bool m_logged_port_a_not_connected;
	bool m_logged_port_b_not_connected;
	bool m_logged_ca1_not_connected;
	bool m_logged_ca2_not_connected;
	bool m_logged_cb1_not_connected;
	bool m_logged_cb2_not_connected;
};


// device type definition
DECLARE_DEVICE_TYPE(PIA6821, pia6821_device)


#endif // MAME_DEVICES_MACHINE_6821PIA_H
