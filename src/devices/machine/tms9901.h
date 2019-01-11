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

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	template <class Object> devcb_base &set_readblock_callback(Object &&cb)  { return m_read_block.set_callback(std::forward<Object>(cb)); }
	template <unsigned N, class Object> devcb_base &set_p_callback(Object &&cb)  { return m_write_p[N].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_intlevel_callback(Object &&cb)  { return m_interrupt.set_callback(std::forward<Object>(cb)); }

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

	// State of the INT1-INT15 lines (must be inverted when queried)
	// Note that the levels must also be delivered when reading the pins, which
	// may require to latch the int levels.
	int m_int_state;
	int m_old_int_state;        // stores the previous value to avoid useless INT line assertions
	int m_enabled_ints;         // interrupt enable mask

	bool m_int_pending;         // status of the int* pin (connected to TMS9900)
	bool m_timer_int_pending;   // timer int pending (overrides int3 pin if timer enabled)

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

	// MESS timer, used to emulate the decrementer register
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

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TMS9901_READBLOCK_HANDLER( _read ) \
	devcb = &downcast<tms9901_device &>(*device).set_readblock_callback(DEVCB_##_read);

#define MCFG_TMS9901_P0_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<0>(DEVCB_##_write);

#define MCFG_TMS9901_P1_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<1>(DEVCB_##_write);

#define MCFG_TMS9901_P2_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<2>(DEVCB_##_write);

#define MCFG_TMS9901_P3_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<3>(DEVCB_##_write);

#define MCFG_TMS9901_P4_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<4>(DEVCB_##_write);

#define MCFG_TMS9901_P5_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<5>(DEVCB_##_write);

#define MCFG_TMS9901_P6_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<6>(DEVCB_##_write);

#define MCFG_TMS9901_P7_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<7>(DEVCB_##_write);

#define MCFG_TMS9901_P8_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<8>(DEVCB_##_write);

#define MCFG_TMS9901_P9_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<9>(DEVCB_##_write);

#define MCFG_TMS9901_P10_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<10>(DEVCB_##_write);

#define MCFG_TMS9901_P11_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<11>(DEVCB_##_write);

#define MCFG_TMS9901_P12_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<12>(DEVCB_##_write);

#define MCFG_TMS9901_P13_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<13>(DEVCB_##_write);

#define MCFG_TMS9901_P14_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<14>(DEVCB_##_write);

#define MCFG_TMS9901_P15_HANDLER( _write ) \
	devcb = &downcast<tms9901_device &>(*device).set_p_callback<15>(DEVCB_##_write);

#define MCFG_TMS9901_INTLEVEL_HANDLER( _intlevel ) \
	devcb = &downcast<tms9901_device &>(*device).set_intlevel_callback(DEVCB_##_intlevel);

#endif // MAME_MACHINE_TMS9901_H
