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

/* Masks for the interrupts levels available on TMS9901 */
#define TMS9901_INT1 0x0002
#define TMS9901_INT2 0x0004
#define TMS9901_INT3 0x0008     // overriden by the timer interrupt
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

struct tms9901_interface
{
	int                 interrupt_mask;         // a bit for each input pin whose state is always notified to the TMS9901 core
	devcb_read8         read_handler;           // 4*8 bits, to be selected using the offset (0-3)
	devcb_write_line    write_handler[16];      // 16 Pn outputs
	devcb_write8        interrupt_callback;     // called when interrupt bus state changes
};

class tms9901_device : public device_t
{
public:
	tms9901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_single_int(int pin_number, int state);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

private:
	static const device_timer_id DECREMENTER = 0;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	void timer_reload(void);
	void field_interrupts(void);

	void device_start(void);
	void device_stop(void);
	void device_reset(void);

	/* interrupt registers */
	// mask:  bit #n is set if pin #n is supported as an interrupt pin,
	// i.e. the driver sends a notification whenever the pin state changes
	// setting these bits is not required, but it saves you the trouble of
	// saving the state of interrupt pins and feeding it to the port read
	// handlers again
	int m_supported_int_mask;
	int m_int_state;            // state of the int1-int15 lines (must be inverted when queried)
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

	// Callbacks
	devcb_resolved_read8        m_read_block;
	devcb_resolved_write_line   m_write_line[16];
	devcb_resolved_write8       m_interrupt;  // also delivers the interrupt level
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TMS9901_ADD(_tag, _intrf, _rate) \
	MCFG_DEVICE_ADD(_tag, TMS9901, _rate) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __TMS9901_H__ */
