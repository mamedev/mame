// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    TMS9901 Programmable System Interface

                    +--------------+
               RST1*| 1   |  |   40| Vcc
             CRUOUT | 2   +--+   39| S0
             CRUCLK | 3          38| P0
              CRUIN | 4          37| P1
                 CE*| 5          36| S1
               INT6*| 6          35| S2
               INT5*| 7          34| INT7*  / P15
               INT4*| 8          33| INT8*  / P14
               INT3*| 9          32| INT9*  / P13
                Phi*|10          31| INT10* / P12
             INTREQ*|11          30| INT11* / P11
                IC3 |12          29| INT12* / P10
                IC2 |13          28| INT13* / P9
                IC1 |14          27| INT14* / P8
                IC0 |15          26| P2
                Vss |16          25| S3
               INT1*|17          24| S4
               INT2*|18          23| INT15* / P7
                 P6 |19          22| P3
                 P5 |20          21| P4
                    +--------------+

Overview:
    TMS9901 is a support chip for TMS9900.  It handles interrupts, provides
    several I/O pins, and a timer (a.k.a. clock: it is merely a register which
    decrements regularly and can generate an interrupt when it reaches 0).

    It communicates with the TMS9900 with the CRU bus, and with the rest of the
    world with a number of parallel I/O pins.

    I/O and timer functions should work with any other 990/99xx/99xxx CPU.
    On the other hand, interrupt handling was primarily designed for tms9900
    and 99000 based systems: other CPUs can support interrupts, but not the 16
    distinct interrupt vectors.

Pins:
    Vcc, Vss: power supply
    Phi*: system clock (connected to TMS9900 Phi3* or TMS9980 CLKOUT*)
    RST1*: reset input
    CRUIN, CRUOUT, CRUCLK, CE*, S0-S4: CRU bus (CPU interface)
    INTREQ*, IC0-IC3: interrupt bus (CPU interface)
    INT*1-INT*6: used as interrupt/input pins.
    P0-P6: used as input/output pins.
    INT*7/P15-INT*15/P7: used as either interrupt/input or input/output pins.
      Note that a pin cannot be used simultaneously as output and as interrupt.
      (This is mostly obvious, but it implies that you cannot trigger an
      interrupt by setting the output state of a pin, which is not SO obvious.)

Interrupt handling:
    After each clock cycle, TMS9901 latches the state of INT1*-INT15* (except
    pins which are set as output pins).  If the clock is enabled, it replaces
    INT3* with an internal timer interrupt flag.  Then it inverts the value and
    performs a bit-wise AND with the interrupt mask.

    If there are some unmasked interrupt bits, INTREQ* is asserted and the code
    of the lowest active interrupt is placed on IC0-IC3.  If these pins are
    duly connected to the tms9900 INTREQ* and IC0-IC3 pins, the result is that
    asserting an INTn* on tms9901 will cause a level-n interrupt request on the
    tms9900, provided that this interrupt pin is not masked in tms9901, and
    that no unmasked higher-priority (i.e. lower-level) interrupt pin is set.

    This interrupt request lasts for as long as the interrupt pin and the
    relevant bit in the interrupt mask are set (level-triggered interrupts).
    (The request may be shadowed by a higher-priority interrupt request, but
    it will resume when the higher-priority request ends.)

    TIMER interrupts are kind of an exception, since they are not associated
    with an external interrupt pin.  I think there is an internal timer
    interrupt flag that is set when the decrementer reaches 0, and is cleared
    by a write to the 9901 int*3 enable bit ("SBO 3" in interrupt mode).

TODO:
    * Emulate the RST1* input.  Note that RST1* active (low) makes INTREQ*
      inactive (high) with IC0-IC3 = 0.
    * the clock read register is updated every time the timer decrements when
      the TMS9901 is not in clock mode.  This probably implies that if the
      clock mode is cleared and re-asserted immediately, the tms9901 may fail
      to update the clock read register: this is not emulated.
    * The clock mode is entered when a 1 is written to the control bit.  It is
      exited when a 0 is written to the control bit or the a tms9901 select bit
      greater than 15 is accessed.  According to the data sheet, "when CE* is
      inactive (HIGH), the PSI is not disabled from seeing the select lines.
      As the CPU is accessing memory, A10-A14 could very easily have a value of
      15 or greater" (this is assuming that S0-S4 are connected to A10-A14,
      which makes sense with most tms9900 family members).  There is no way
      this "feature" (I would call it a hardware bug) can be emulated
      efficiently, as we would need to watch every memory access.

MZ: According to the description in
       A. Osborne, G. Kane: Osborne 16-bit microprocessor handbook
       page 3-81
    the 9901 only temporarily leaves the timer mode as long as S0 is set to 1.
    In the meantime the timer function continues but cannot be queried. This
    makes it possible to continue using the chip as a timer while working with
    its I/O pins. Thus I believe the above TODO concering the exit of the timer
    mode is not applicable.
    The problem is that the original 9901 specification is not clear about this.

MZ: Turned to class (January 2012)

TODO: Tests on a real machine
- Set an interrupt input (e.g. keyboard for Geneve), trigger RST2*, check whether
  interrupt mask has been reset
- Check whether the clock_read_register is updated whenever clock mode is exited
  (in particular when S0=1, i.e. A10=1 -> addresses xxxx xxxx xx1x xxxx
  requires to write a program that fits into 32 bytes; workspace elsewhere)

    Raphael Nabet, 2000-2004
    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#include <math.h>
#include "emu.h"

#include "tms9901.h"

/*
    Debugging flags.
*/
#define TRACE_PINS 0
#define TRACE_CLOCK 0
#define TRACE_MODE 0

/*
    Constructor
*/
tms9901_device::tms9901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, TMS9901, "TMS9901 Programmable System Interface", tag, owner, clock, "tms9901", __FILE__),
	m_read_block(*this),
	m_write_p0(*this),
	m_write_p1(*this),
	m_write_p2(*this),
	m_write_p3(*this),
	m_write_p4(*this),
	m_write_p5(*this),
	m_write_p6(*this),
	m_write_p7(*this),
	m_write_p8(*this),
	m_write_p9(*this),
	m_write_p10(*this),
	m_write_p11(*this),
	m_write_p12(*this),
	m_write_p13(*this),
	m_write_p14(*this),
	m_write_p15(*this),
	m_interrupt(*this)
{
}

/*
    should be called after any change to int_state or enabled_ints.
*/
void tms9901_device::field_interrupts(void)
{
	int current_ints;

	// m_int_state: inverted state of lines INT1*-INT15*. Bits are set by set_single_int only.
	current_ints = m_int_state;
	if (m_clock_register != 0)
	{
		// if timer is enabled, INT3 pin is overridden by timer
		if (m_timer_int_pending)
		{
			if (TRACE_CLOCK) logerror("%s: timer fires\n", tag());
			current_ints |= TMS9901_INT3;
		}
		else
		{
			if (TRACE_CLOCK) logerror("%s: timer clear\n", tag());
			current_ints &= ~TMS9901_INT3;
		}
	}

	// enabled_ints: enabled interrupts
	// Remove all settings from pins that are set as outputs (INT7*-INT15* share the same pins as P15-P7)
	current_ints &= m_enabled_ints & (~m_pio_direction_mirror);

	// Check whether we have a new state. For systems that use level-triggered
	// interrupts it should not do any harm if the line is re-asserted
	// but we may as well avoid this.
	if (current_ints == m_old_int_state)
		return;

	m_old_int_state = current_ints;

	if (current_ints != 0)
	{
		// find which interrupt tripped us:
		// the number of the first (i.e. least significant) non-zero bit among
		// the 16 first bits
		// we simply look for the first bit set to 1 in current_ints...
		int level = 0;

		while ((current_ints & 1)==0)
		{
			current_ints >>= 1; /* try next bit */
			level++;
		}
		m_int_pending = true;
		if (!m_interrupt.isnull())
			m_interrupt(level, 1, 0xff);  // the offset carries the IC0-3 level
	}
	else
	{
		m_int_pending = false;
		if (!m_interrupt.isnull())
			m_interrupt(0xf, 0, 0xff);  //Spec: INTREQ*=1 <=> IC0,1,2,3 = 1111
	}
}

/*
    function which should be called by the driver when the state of an INTn*
    pin changes (only required if the pin is set up as an interrupt pin)

    state == CLEAR_LINE: INTn* is inactive (high)
    state == ASSERT_LINE: INTn* is active (low)

    0<=pin_number<=15
*/
void tms9901_device::set_single_int(int pin_number, int state)
{
	/* remember new state of INTn* pin state */
	if (state==ASSERT_LINE)
		m_int_state |= 1 << pin_number;
	else
		m_int_state &= ~(1 << pin_number);

	field_interrupts();
}

/*
    load the content of m_clock_register into the decrementer
*/
void tms9901_device::timer_reload(void)
{
	if (m_clock_register != 0)
	{   /* reset clock interval */
		m_decrementer_value = m_clock_register;
		m_decrementer->enable(true);
	}
	else
	{   /* clock interval == 0 -> no timer */
		m_decrementer->enable(false);
	}
}

/*----------------------------------------------------------------
    TMS9901 CRU interface.
----------------------------------------------------------------*/

/*
    Read a 8 bit chunk from tms9901.

    signification:
    bit 0: m_clock_mode
    if (m_clock_mode == false)
     bit 1-15: current status of the INT1*-INT15* pins
    else
     bit 1-14: current timer value
     bit 15: value of the INTREQ* (interrupt request to TMS9900) pin.

    bit 16-31: current status of the P0-P15 pins (quits timer mode, too...)
*/
READ8_MEMBER( tms9901_device::read )
{
	int answer = 0;

	offset &= 0x003;

	switch (offset)
	{
	case 0:
		if (m_clock_mode)
		{
			// Clock mode. The LSB reflects the CB bit which is set to 1 for clock mode.
			answer = ((m_clock_read_register & 0x7F) << 1) | 0x01;
		}
		else
		{
			// Interrupt mode
			// Note that we rely on the read function to deliver the same
			// INTx levels that have been signaled via the set_single_int method.
			// This may mean that those levels must be latched by the callee.
			if (!m_read_block.isnull())
				answer |= m_read_block(TMS9901_CB_INT7);

			// Remove the bits that are set as outputs (can only be INT7*)
			answer &= ~m_pio_direction_mirror;

			// Set those bits here
			answer |= (m_pio_output_mirror & m_pio_direction_mirror) & 0xFF;
		}
		if (TRACE_PINS) logerror("%s: input on lines INT7..CB = %02x\n", tag(), answer);
		break;
	case 1:
		if (m_clock_mode)
		{
			// clock mode
			answer = (m_clock_read_register & 0x3F80) >> 7;
			if (!m_int_pending)
				answer |= 0x80;
		}
		else
		{
			// See above concerning the INT levels.
			if (!m_read_block.isnull())
				answer |= m_read_block(TMS9901_INT8_INT15);

			// Remove the bits that are set as outputs (can be any line)
			answer &= ~(m_pio_direction_mirror >> 8);
			answer |= (m_pio_output_mirror & m_pio_direction_mirror) >> 8;
		}
		if (TRACE_PINS) logerror("%s: input on lines INT15..INT8 = %02x\n", tag(), answer);
		break;
	case 2:
		/* exit timer mode */
		// MZ: See comments at the beginning. I'm sure that we do not quit clock mode.
		// m_clock_mode = false;

		if (!m_read_block.isnull())
			answer = m_read_block(TMS9901_P0_P7);
		else
			answer = 0;

		answer &= ~m_pio_direction;
		answer |= (m_pio_output & m_pio_direction) & 0xFF;
		if (TRACE_PINS) logerror("%s: input on lines P7..P0 = %02x\n", tag(), answer);

		break;
	case 3:
		// MZ: see above
		// m_clock_mode = false;
		if (!m_read_block.isnull())
			answer = m_read_block(TMS9901_P8_P15);
		else
			answer = 0;

		answer &= ~(m_pio_direction >> 8);
		answer |= (m_pio_output & m_pio_direction) >> 8;
		if (TRACE_PINS) logerror("%s: input on lines P15..P8 = %02x\n", tag(), answer);

		break;
	}

	return answer;
}

/*
    Write 1 bit to tms9901.

    signification:
    bit 0: write m_clock_mode
    if (!m_clock_mode)
     bit 1-15: write interrupt mask register
    else
     bit 1-14: write timer period
     bit 15: if written value == 0, soft reset (just resets all I/O pins as input)

    bit 16-31: set output state of P0-P15 (and set them as output pin) (quit timer mode, too...)
*/
WRITE8_MEMBER ( tms9901_device::write )
{
	data &= 1;  /* clear extra bits */
	offset &= 0x01F;

	if (offset >= 0x10)
	{
		int pin = offset & 0x0F;
		if (TRACE_PINS) logerror("%s: output on P%d = %d\n", tag(), pin, data);

		int bit = (1 << pin);

		// MZ: see above - I think this is wrong
		// m_clock_mode = false; // exit timer mode

		// Once a value is written to a pin, the pin remains in output mode
		// until the chip is reset
		m_pio_direction |= bit;

		// Latch the value
		if (data)
			m_pio_output |= bit;
		else
			m_pio_output &= ~bit;

		if (pin >= 7)
		{
			// pins P7-P15 are mirrored as INT15*-INT7*,
			// also using the same pins in the package
			int mirror_bit = (1 << (22 - pin));

			// See above
			m_pio_direction_mirror |= mirror_bit;

			if (data)
				m_pio_output_mirror |= mirror_bit;
			else
				m_pio_output_mirror &= ~mirror_bit;
		}

		switch (offset)
		{
		case 0x10:
			if (!m_write_p0.isnull()) m_write_p0(data); break;
		case 0x11:
			if (!m_write_p1.isnull()) m_write_p1(data); break;
		case 0x12:
			if (!m_write_p2.isnull()) m_write_p2(data); break;
		case 0x13:
			if (!m_write_p3.isnull()) m_write_p3(data); break;
		case 0x14:
			if (!m_write_p4.isnull()) m_write_p4(data); break;
		case 0x15:
			if (!m_write_p5.isnull()) m_write_p5(data); break;
		case 0x16:
			if (!m_write_p6.isnull()) m_write_p6(data); break;
		case 0x17:
			if (!m_write_p7.isnull()) m_write_p7(data); break;
		case 0x18:
			if (!m_write_p8.isnull()) m_write_p8(data); break;
		case 0x19:
			if (!m_write_p9.isnull()) m_write_p9(data); break;
		case 0x1A:
			if (!m_write_p10.isnull()) m_write_p10(data); break;
		case 0x1B:
			if (!m_write_p11.isnull()) m_write_p11(data); break;
		case 0x1C:
			if (!m_write_p12.isnull()) m_write_p12(data); break;
		case 0x1D:
			if (!m_write_p13.isnull()) m_write_p13(data); break;
		case 0x1E:
			if (!m_write_p14.isnull()) m_write_p14(data); break;
		case 0x1F:
			if (!m_write_p15.isnull()) m_write_p15(data); break;

		}
		return;
	}

	if (offset == 0)
	{
		// Write to control bit (CB)
		if (data == 0)
		{
			// Switch to interrupt mode; quit clock mode
			m_clock_mode = false;
			if (TRACE_MODE) logerror("%s: int mode\n", tag());
		}
		else
		{
			m_clock_mode = true;
			if (TRACE_MODE) logerror("%s: clock mode\n", tag());
			// we are switching to clock mode: latch the current value of
			// the decrementer register
			if (m_clock_register != 0)
				m_clock_read_register = m_decrementer_value;
			else
				m_clock_read_register = 0;      /* timer inactive... */
		}
	}
	else
	{
		if (offset == 0x0f)
		{
			if (m_clock_mode)
			{   /* in clock mode this is the soft reset bit */
				if (!data)
				{   // TMS9901 soft reset (RST2*)
					// Spec: "Writing a 0 to bit 15 while in the clock mode executes a soft reset on the I/O pins.
					// [...] RST2* will program all ports to the input mode"
					m_pio_direction = 0;
					m_pio_direction_mirror = 0;

					// "RST1* (power-up reset) will reset all mask bits low."
					// Spec is not clear on whether the mask bits are also reset by RST2*
					// TODO: Check on a real machine. (I'd guess from the text they are not touched)
					m_enabled_ints = 0;
					if (TRACE_MODE) logerror("%s: Soft reset (RST2*)\n", tag());
				}
			}
			else
			{   /* modify interrupt enable mask */
				if (data)
					m_enabled_ints |= 0x4000;       /* set bit */
				else
					m_enabled_ints &= ~0x4000;      /* unset bit */

				if (TRACE_PINS) logerror("%s: interrupts = %04x\n", tag(), m_enabled_ints);
				field_interrupts();     /* changed interrupt state */
			}
		}
		else
		{
			// write one bit to 9901 (bits 1-14)
			//
			// m_clock_mode==false ?  Disable/Enable an interrupt
			// :  Bit in clock interval
			//
			// offset is the index of the modified bit of register (-> interrupt number -1)
			if (m_clock_mode)
			{   /* modify clock interval */
				int bit = 1 << ((offset & 0x0F) - 1);  /* corresponding mask */

				if (data)
					m_clock_register |= bit;       /* set bit */
				else
					m_clock_register &= ~bit;      /* clear bit */

				/* reset clock timer (page 8) */
				if (TRACE_CLOCK) logerror("%s: clock register = %04x\n", tag(), m_clock_register);
				timer_reload();
			}
			else
			{   /* modify interrupt enable mask */
				int bit = 1 << (offset & 0x0F);    /* corresponding mask */

				if (data)
					m_enabled_ints |= bit;     /* set bit */
				else
					m_enabled_ints &= ~bit;        /* unset bit */

				if (offset == 3)
					m_timer_int_pending = false;    /* SBO 3 clears pending timer interrupt (??) */

				if (TRACE_MODE) logerror("%s: enabled interrupts = %04x\n", tag(), m_enabled_ints);
				field_interrupts();     /* changed interrupt state */
			}
		}
	}
}

/*
    Timer callback
    Decrementer counts down the value set in clock mode; when it reaches 0,
    raises an interrupt and resets to the start value
    The decrementer works as long as the clock_register contains a non-zero value.
*/
void tms9901_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id==DECREMENTER) // we have only that one
	{
		m_decrementer_value--;
		if (TRACE_CLOCK) logerror("%s: decrementer = %d\n", tag(), m_decrementer_value);
		if (m_decrementer_value<=0)
		{
			m_timer_int_pending = true;         // decrementer interrupt requested
			field_interrupts();
			m_decrementer_value = m_clock_register;
		}
	}
}

/*-------------------------------------------------
    device_stop - device-specific stop
-------------------------------------------------*/

void tms9901_device::device_stop(void)
{
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void tms9901_device::device_reset(void)
{
	m_timer_int_pending = false;
	m_enabled_ints = 0;

	m_pio_direction = 0;
	m_pio_direction_mirror = 0;
	m_pio_output = m_pio_output_mirror = 0;

	// This is an interrupt level latch, positive logic (bit 0 = no int)
	// The inputs are negative logic (INTx*)
	m_int_state = 0;

	m_old_int_state = -1;
	field_interrupts();

	m_clock_mode = false;

	m_clock_register = 0;
	timer_reload();
}


/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void tms9901_device::device_start(void)
{
	m_decrementer = timer_alloc(DECREMENTER);
	m_decrementer->adjust(attotime::from_hz(clock() / 64.), 0, attotime::from_hz(clock() / 64.));
	m_decrementer->enable(false);

	m_read_block.resolve();
	m_write_p0.resolve();
	m_write_p1.resolve();
	m_write_p2.resolve();
	m_write_p3.resolve();
	m_write_p4.resolve();
	m_write_p5.resolve();
	m_write_p6.resolve();
	m_write_p7.resolve();
	m_write_p8.resolve();
	m_write_p9.resolve();
	m_write_p10.resolve();
	m_write_p11.resolve();
	m_write_p12.resolve();
	m_write_p13.resolve();
	m_write_p14.resolve();
	m_write_p15.resolve();
	m_interrupt.resolve();

	m_clock_register = 0;
}

const device_type TMS9901 = &device_creator<tms9901_device>;
