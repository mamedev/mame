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
	// Masks for the interrupts levels available on TMS9901
	static constexpr int INT1 = 0x0002;
	static constexpr int INT2 = 0x0004;
	static constexpr int INT3 = 0x0008;     // overridden by the timer interrupt
	static constexpr int INT4 = 0x0010;
	static constexpr int INT5 = 0x0020;
	static constexpr int INT6 = 0x0040;
	static constexpr int INT7 = 0x0080;
	static constexpr int INT8 = 0x0100;
	static constexpr int INT9 = 0x0200;
	static constexpr int INTA = 0x0400;
	static constexpr int INTB = 0x0800;
	static constexpr int INTC = 0x1000;
	static constexpr int INTD = 0x2000;
	static constexpr int INTE = 0x4000;
	static constexpr int INTF = 0x8000;

	enum
	{
		CB_INT7 = 0,
		INT8_INT15 = 1,
		P0_P7 = 2,
		P8_P15 = 3
	};

	tms9901_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_single_int(int pin_number, int state);

	DECLARE_WRITE_LINE_MEMBER( rst1_line );

	// Synchronous clock input
	DECLARE_WRITE_LINE_MEMBER( phi_line );

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	auto p_out_cb(int n) { return m_write_p[n].bind(); }
	auto read_cb() { return m_read_block.bind(); }
	auto intlevel_cb() { return m_interrupt.bind(); }

	uint16_t pio_outputs() const { return m_pio_output | ~m_pio_direction; }

private:
	static constexpr device_timer_id DECREMENTER = 0;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void timer_reload();
	void field_interrupts();

	void device_start() override;
	void device_stop() override;
	void device_reset() override;

	// Common method for device_reset and rst1_line
	void do_reset();

	// Common clock handling
	void clock_in(line_state clk);

	// State of the INT1-INT15 lines (must be inverted when queried)
	// Note that the levels must also be delivered when reading the pins, which
	// may require to latch the int levels.
	int m_int_state;
	int m_old_int_state;        // stores the previous value to avoid useless INT line assertions
	int m_enabled_ints;         // interrupt enable mask

	bool m_int_pending;         // status of the int* pin (connected to TMS9900)
	bool m_timer_int_pending;   // timer int pending (overrides int3 pin if timer enabled)

	bool m_clock_active;
	int  m_clockdiv;            // Clock divider counter (for synchronous clock)

	// PIO registers
	int m_pio_direction;        // direction register for PIO

	// current PIO output (to be masked with pio_direction)
	int m_pio_output;

	// mirrors used for INT7*-INT15*
	int m_pio_direction_mirror;
	int m_pio_output_mirror;

	// =======================================================================

	// TMS9901 clock mode
	// false = so-called interrupt mode (read interrupt state, write interrupt enable mask)
	// true = clock mode (read/write clock interval)
	bool m_clock_mode;

	// Timer, used to emulate the decrementer register
	emu_timer *m_decrementer;

	// clock interval, loaded in decrementer when it reaches 0.
	// 0 means decrementer off
	int m_clock_register;

	// Current decrementer value
	int m_decrementer_value;

	// when we go into timer mode, the decrementer is copied there to allow to read it reliably
	int m_clock_read_register;

	// =======================================================================

	// Read callback.
	devcb_read8        m_read_block;

	// I/O lines, used for output. When used as inputs, the levels are delivered via the m_read_block
	devcb_write_line   m_write_p[16];

	// The invocation corresponds to the INTREQ signal (with the level passed as data)
	// and the address delivers the interrupt level (0-15)
	devcb_write8       m_interrupt;
};

#endif // MAME_MACHINE_TMS9901_H
