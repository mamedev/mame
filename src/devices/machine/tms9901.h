// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    TMS9901 Programmable System Interface
    See tms9901.c for documentation

    Raphael Nabet
    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TMS9901_H__
#define __TMS9901_H__

#include "emu.h"

extern const device_type TMS9901;

/***************************************************************************
    MACROS
***************************************************************************/

// Masks for the interrupts levels available on TMS9901

#define TMS9901_INT1 0x0002
#define TMS9901_INT2 0x0004
#define TMS9901_INT3 0x0008     // overridden by the timer interrupt
#define TMS9901_INT4 0x0010
#define TMS9901_INT5 0x0020
#define TMS9901_INT6 0x0040
#define TMS9901_INT7 0x0080
#define TMS9901_INT8 0x0100
#define TMS9901_INT9 0x0200
#define TMS9901_INTA 0x0400
#define TMS9901_INTB 0x0800
#define TMS9901_INTC 0x1000
#define TMS9901_INTD 0x2000
#define TMS9901_INTE 0x4000
#define TMS9901_INTF 0x8000

enum
{
	TMS9901_CB_INT7 = 0,
	TMS9901_INT8_INT15 = 1,
	TMS9901_P0_P7 = 2,
	TMS9901_P8_P15 = 3
};

/***************************************************************************
    CLASS DEFINITION
***************************************************************************/

class tms9901_device : public device_t
{
public:
	tms9901_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void set_single_int(int pin_number, int state);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	template<class _Object> static devcb_base &static_set_readblock_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_read_block.set_callback(object); }

	template<class _Object> static devcb_base &static_set_p0_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p0.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p1_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p1.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p2_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p2.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p3_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p3.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p4_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p4.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p5_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p5.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p6_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p6.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p7_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p7.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p8_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p8.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p9_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p9.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p10_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p10.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p11_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p11.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p12_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p12.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p13_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p13.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p14_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p14.set_callback(object); }
	template<class _Object> static devcb_base &static_set_p15_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_write_p15.set_callback(object); }

	template<class _Object> static devcb_base &static_set_intlevel_callback(device_t &device, _Object object)  { return downcast<tms9901_device &>(device).m_interrupt.set_callback(object); }

private:
	static const device_timer_id DECREMENTER = 0;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void timer_reload(void);
	void field_interrupts(void);

	void device_start(void) override;
	void device_stop(void) override;
	void device_reset(void) override;

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
	devcb_write_line   m_write_p0;
	devcb_write_line   m_write_p1;
	devcb_write_line   m_write_p2;
	devcb_write_line   m_write_p3;
	devcb_write_line   m_write_p4;
	devcb_write_line   m_write_p5;
	devcb_write_line   m_write_p6;
	devcb_write_line   m_write_p7;
	devcb_write_line   m_write_p8;
	devcb_write_line   m_write_p9;
	devcb_write_line   m_write_p10;
	devcb_write_line   m_write_p11;
	devcb_write_line   m_write_p12;
	devcb_write_line   m_write_p13;
	devcb_write_line   m_write_p14;
	devcb_write_line   m_write_p15;

	// The invocation corresponds to the INTREQ signal (with the level passed as data)
	// and the address delivers the interrupt level (0-15)
	devcb_write8       m_interrupt;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TMS9901_READBLOCK_HANDLER( _read ) \
	devcb = &tms9901_device::static_set_readblock_callback( *device, DEVCB_##_read );

#define MCFG_TMS9901_P0_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p0_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P1_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p1_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P2_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p2_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P3_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p3_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P4_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p4_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P5_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p5_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P6_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p6_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P7_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p7_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P8_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p8_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P9_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p9_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P10_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p10_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P11_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p11_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P12_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p12_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P13_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p13_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P14_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p14_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_P15_HANDLER( _write ) \
	devcb = &tms9901_device::static_set_p15_callback( *device, DEVCB_##_write );

#define MCFG_TMS9901_INTLEVEL_HANDLER( _intlevel ) \
	devcb = &tms9901_device::static_set_intlevel_callback( *device, DEVCB_##_intlevel );

#endif /* __TMS9901_H__ */
