// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    TMS9901 Programmable System Interface
    See tms9901.c for documentation

    Raphael Nabet
    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_MACHINE_TMS9901_H
#define MAME_MACHINE_TMS9901_H


DECLARE_DEVICE_TYPE(TMS9901, tms9901_device)

/***************************************************************************
    MACROS
***************************************************************************/

/***************************************************************************
    CLASS DEFINITION
***************************************************************************/

class tms9901_device : public device_t
{
public:
	// I/O pins
	enum
	{
		INT1=1,
		INT2,
		INT3,
		INT4,
		INT5,
		INT6,
		INT7_P15,
		INT8_P14,
		INT9_P13,
		INT10_P12,
		INT11_P11,
		INT12_P10,
		INT13_P9,
		INT14_P8,
		INT15_P7,
		P0,
		P1,
		P2,
		P3,
		P4,
		P5,
		P6
	} pins;

	tms9901_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_int_line(int pin_number, int state);

	void rst1_line(int state);

	// Synchronous clock input
	void phi_line(int state);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	bool read_bit(int bit);
	void write_bit(int bit, bool set);

	auto p_out_cb(int n) { return m_write_p[n].bind(); }
	auto read_cb() { return m_read_port.bind(); }
	auto intreq_cb() { return m_interrupt.bind(); }

	// Pins IC3...IC0
	// When no interrupt is active, the IC lines are all set to 1
	// The difference to /INT15 is that INTREQ is cleared.
	int get_int_level() { return m_int_pending? m_int_level : 15; }

	// Return PIO all outputs; ports configured as inputs return 1
	// Used by si5500
	uint16_t pio_outputs() const { return m_pio_output | ~m_pio_direction; }

	void set_poll_int_lines(bool poll) { m_poll_lines = poll; }

	void update_clock();

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(decrement_tick);

	void soft_reset();

	// Common method for device_reset and rst1_line
	void do_reset();

	// Common clock handling
	void timer_clock_in(line_state clk);

	// Direction and value of P pins
	bool is_output(int p);
	bool output_value(int p);
	void set_and_latch_output(int p, bool val);
	void set_bit(uint16_t& bitfield, int pos, bool val);

	int m_int_level;
	int m_last_level;

	// State of the INT1-INT15 lines (must be inverted when queried)
	// Note that the levels must also be delivered when reading the pins, which
	// may require to latch the int levels on the caller's side.
	uint16_t m_int_line;
	uint16_t m_int_mask;
	bool m_int_pending;
	bool m_poll_lines;

	// Find the interrupt with the lowest level (most important)
	void prioritize_interrupts();

	// Outgoing INTREQ* line
	void signal_int();

	// Sample the interrupt inputs.
	void sample_interrupt_inputs();

	// P15..P0; 1=output H, 0=output L
	uint16_t m_pio_output;

	// For P15..P0; 1=output, 0=input
	// Once set to 1, a reset is necessary to return the pin to input
	uint16_t m_pio_direction;

	// =======================================================================

	// TMS9901 clock mode
	// false = interrupt mode (read interrupt state, write interrupt enable mask)
	// true = clock mode (read/write clock interval)
	bool m_clock_mode;

	// Clock divider
	int  m_clockdiv;

	// Clock has reached 0
	bool m_timer_int_pending;

	// Timer, used to emulate the decrementer register
	emu_timer *m_decrementer;

	// Clock interval, loaded in decrementer when it reaches 0.
	uint16_t m_clock_register;

	// Current decrementer value
	uint16_t m_decrementer_value;

	// when we go into timer mode, the decrementer is copied there to allow to read it reliably
	uint16_t m_clock_read_register;

	// =======================================================================

	// Read callback.
	devcb_read8        m_read_port;

	// I/O lines, used for output. When used as inputs, the levels are delivered via the m_read_block
	devcb_write_line::array<16> m_write_p;

	// INTREQ pin
	devcb_write_line   m_interrupt;
};

#endif // MAME_MACHINE_TMS9901_H
