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

Reference: [1] TMS9901 Programmable Systems Interface Data Manual, July 1977
           [2] A. Osborne, G. Kane: Osborne 16-bit microprocessor handbook

Overview:
    TMS9901 is a support chip for TMS9900.  It handles interrupts, provides
    several I/O pins, and a timer, which is a register which
    decrements continuously and can be set to generate an interrupt when it
    reaches 0.

    It communicates with the TMS9900 with the CRU bus, and with the rest of the
    world with a number of parallel I/O pins.

    I/O and timer functions should work with any other 990/99xx/99xxx CPU.
    On the other hand, interrupt handling was primarily designed for TMS9900
    and 99000 based systems: other CPUs can support interrupts, but not the 16
    distinct interrupt vectors.

    For the 9980A, the IC1-IC3 lines are connected to the IC0-IC2 lines of the
    9980A.

Pins:
    Vcc, Vss: power supply
    Phi* :    system clock (connected to TMS9900 Phi3* or TMS9980A CLKOUT*)
    RST1*:    Reset input
    CRUIN,
    CRUOUT,
    CRUCLK:   CRU bus
    CE*:      Chip enable; typically driven by a decoder for the CRU address
    S0-S4:    CRU access bits (0..31; S0 is MSB)
    INTREQ*:  Interrupt request; active (0) when an interrupt is signaled to the CPU
    IC0-IC3:  Interrupt level (0..15, IC0 is MSB)

    Three groups of I/O pins:

    Group 1: INT1*-INT6*: Interrupt inputs.
    Group 2: INT7*_P15 - INT15*_P7: Interrupt inputs or I/O ports
    Group 3: P0-P6: I/O ports

    In group 2, the interrupt inputs and I/O ports share their pins, which
    leads to mirroring in the CRU address space.

Input/Output ports:
    P0 to P15 are preconfigured as input ports. By writing a value to a port,
    it is configured as an output. Caution must be taken that the pin is not
    fed with some logic level when setting it as output, because this may
    damage the port. To reconfigure it as an input, the chip must be reset
    by the hard RST1* line or the soft RST2* operation (setting bit 15 to 0
    in clock mode). Output pins can be read and return the currently set value.

Interrupt inputs (group 1 and 2)
    The interrupt inputs (INT1*-INT15*) are sampled on each falling edge of
    the phi* clock. An interrupt mask is applied to mask away levels that
    shall not trigger an interrupt. The mask can be set using the SBO/SBZ
    commands (1=arm, 0=disarm) on each of the 15 bits. A disarmed interrupt
    input can still be read like a normal input port via CRU access.

    After each clock cycle, the TMS9901 latches the state of INT1*-INT15*
    (except those pins which are set as output pins).

    If there are some unmasked interrupt bits, INTREQ* is asserted and the code
    of the lowest active interrupt is placed on IC0-IC3. The interrupt line
    and the level lines should be connected to the respective inputs of the
    TMS9900 or TMS9980A.

Group 2 pins (shared I/O and INT*)
    Pins of group 2 are shared between the I/O ports and the interrupt inputs.
    Internally, they are treated as different signals: There are interrupt
    mask bits for INT7*..INT15*, and there is a CRU bit for each of the I/O
    ports. For example, INT7* can be read by bit 7, and the same pin can be
    read as P15 via bit 31. When setting bit 7 to 1, the INT7* input is armed
    and triggers an interrupt at level 7 when asserted.

    In contrast, when writing to bit 31, P15 (same pin) is configured as an
    output, and the written value appears on the pin.

    According to [1], the interrupt mask should be set to 0 for those group 2
    pins that are used as input/output pins so that no unwanted interrupts are
    triggered.

Clock mode:
    The "clock mode" is entered by setting bit 0 to 1; setting to 0 enters
    "interrupt mode". The internal clock is a 14-bit decrementer that
    counts down by 1 every 64 clock ticks. On every update, the value is copied
    into the read register, but only in interrupt mode. In clock mode, the read
    register is locked so that it can be read without being changed.
    Whenever the counter reaches 0, it is reloaded from the clock register on
    the next update.
    Setting the clock register is possible via CRU addresses 1 to 14 in clock
    mode, with bit 1 being the LSB and bit 14 being the MSB. On each bit write
    operation, the current state of the clock register is copied into the counter.


                           Interrupt
                              ^
                              |
    [Clock register]  -> [Decrementer]  ->  [Read register]
         ^                                         |
         |                                         v
         +--<---  CRU write         CRU read---<---+

    The specs somewhat ambiguously say that "writing a non-zero value enables the clock"
    and "the clock is disabled by RST1* or by writing a zero value into the clock register".
    Tests show that when a 0 has been written, the chip still counts down from
    0x3FFF to 0. However, no interrupt is raised when reaching 0, so "enable"
    or "disable" most likely refers to the interrupt.

    When enabled, the clock raises an interrupt level 3 when reaching 0,
    overriding the input from the INT3* input. CRU bit 3 is the mask bit for
    both clock and INT3* input. Writing any value to it changes the mask bit,
    and as a side effect it also clears the clock interrupt.

    According to [2], the clock mode is temporarily left when the CRU bits of
    the I/O ports (bits 16-31) are accessed. Thus, the 9901 can control its
    I/O ports even when it has been set to clock mode before. (Keep in mind
    that clock mode simply means to access the clock register; the clock is
    counting all the time.)

    Raphael Nabet, 2000-2004
    Michael Zapf

*****************************************************************************/

#include "emu.h"
#include "tms9901.h"

#include <cmath>

#define LOG_GENERAL  (1U << 0)
#define LOG_PINS     (1U << 1)
#define LOG_MASK     (1U << 2)
#define LOG_MODE     (1U << 3)
#define LOG_INT      (1U << 4)
#define LOG_CLOCK    (1U << 5)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

/*
    Constructor
*/
tms9901_device::tms9901_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig, TMS9901, tag, owner, clock),
	m_int_level(0),
	m_last_level(0),
	m_int_mask(0),
	m_int_pending(false),
	m_poll_lines(false),
	m_clockdiv(0),
	m_timer_int_pending(false),
	m_read_port(*this),
	m_write_p(*this),
	m_interrupt(*this)
{
}

/*
    Determine the most significant interrupt (lowest number)
*/
void tms9901_device::prioritize_interrupts()
{
	// Prioritizer: Search for the interrupt with the highest level
	bool found = false;

	// Skip the rightmost bit
	uint16_t masked_ints = m_int_line;

	// Is the clock enabled?
	if (m_clock_register != 0)
	{
		// Disable the actual INT3* input
		masked_ints &= ~(1<<INT3);

		// Do we have a clock interrupt?
		if (m_timer_int_pending)
		{
			masked_ints |= (1<<INT3);
			LOGMASKED(LOG_INT, "INT3 (timer) asserted\n");
		}
	}

	m_int_level = 1;
	masked_ints = (masked_ints & m_int_mask)>>1;

	while ((masked_ints!=0) && !found)
	{
		// If INTn is set, stop searching. Consider, however, that
		// within INT7-INT15, those pins configured as outputs are not sampled
		// (shared pins with P15-P7)

		if ((masked_ints & 1) && ((m_int_level < 7) || !is_output(22-m_int_level)))
			found = true;
		else
		{
			m_int_level++;
			masked_ints >>= 1;
		}
	}

	if (!found) m_int_level = 15;

	m_int_pending = found;

	// Only for asynchronous emulation
	if (clock()!=0) signal_int();
}

bool tms9901_device::is_output(int p)
{
	return BIT(m_pio_direction, p);
}

bool tms9901_device::output_value(int p)
{
	return BIT(m_pio_output, p);
}

void tms9901_device::set_and_latch_output(int p, bool val)
{
	set_bit(m_pio_direction, p, true);
	set_bit(m_pio_output, p, val);
	m_write_p[p](val);
}

void tms9901_device::set_bit(uint16_t& bitfield, int pos, bool val)
{
	if (val) bitfield |= (1<<pos);
	else bitfield &= ~(1<<pos);
}

void tms9901_device::signal_int()
{
	if (m_int_level == m_last_level)
		return;

	m_last_level = m_int_level;

	if (m_int_pending)
	{
		LOGMASKED(LOG_INT, "Triggering interrupt, level %d\n", m_int_level);
		if (!m_interrupt.isnull())
			m_interrupt(ASSERT_LINE);
	}
	else
	{
		LOGMASKED(LOG_INT, "Clear all interrupts\n");
		if (!m_interrupt.isnull())
			m_interrupt(CLEAR_LINE);  //Spec: INTREQ*=1 <=> IC0,1,2,3 = 1111
	}
}

/*
    Signal an interrupt line change to the 9901.
    The real circuit samples all active interrupt inputs on every falling edge
    of the phi* clock, which is very inefficient to emulate.

    Accordingly, we let the interrupt producer calls this method and
    so push the interrupt line change.

    state == CLEAR_LINE: INTn* is inactive (high)
    state == ASSERT_LINE: INTn* is active (low)

    n=1..15
*/
void tms9901_device::set_int_line(int n, int state)
{
	if ((n >= 1) && (n <= 15))
	{
		set_bit(m_int_line, n, state==ASSERT_LINE);
		prioritize_interrupts();
	}
}

/*----------------------------------------------------------------
    TMS9901 CRU interface.
----------------------------------------------------------------*/

/*
    Read a bit from tms9901.

     Bit     Meaning
     ---------------
     0       Control bit (0=Interrupt mode, 1=Clock mode)
     1..14   /INTn  (int mode), CLKn (clock mode)
     15      /INT15 (int mode), /INTREQ (clock mode)
     16..31  P0...P15 input

     Reading an output port delivers the latched output value.

     Ports P7 to P15 share pins with the interrupt inputs /INT15 to /INT7
     (in this order). When configured as outputs, reading returns the latched
     values.
*/
uint8_t tms9901_device::read(offs_t offset)
{
	return read_bit(offset)? 0x01 : 0x00;
}

bool tms9901_device::read_bit(int bit)
{
	int crubit = bit & 0x01f;

	if (crubit == 0)
		return m_clock_mode;

	if (crubit > 15)
	{
		// I/O lines
		if (is_output(crubit-16))
			return output_value(crubit-16);
		else
		{
			// Positive logic; should be 0 if there is no connection.
			if (m_read_port.isnull()) return false;
			return m_read_port((crubit<=P6)? crubit : P6+P0-crubit)!=0;
		}
	}

	// If we are here, crubit=1..15
	if (m_clock_mode)
	{
		if (crubit == 15)    // bit 15 in clock mode = /INTREQ
			return !m_int_pending;

		return BIT(m_clock_read_register, crubit-1)!=0;
	}
	else
	{
		// We trust the read_port method to deliver the same INTx levels that
		// have been signaled via the set_int_line method.
		// Thus, those levels must be latched by the component that hosts
		// this 9901. Alternatively, use the interrupt line polling
		// which has a bad impact on performance.
		if (crubit>INT6 && is_output(22-crubit))
			return output_value(22-crubit);
		else
			return m_read_port.isnull()? true : (m_read_port(crubit)!=0);
	}
}


/*
    Write one bit to the tms9901.

     Bit     Meaning
     ----------------------------------------------------------
     0       0=Interrupt mode, 1=Clock mode
     1..14   Clock mode: Set CLKn; Interrupt mode: Set Mask n
     15      Clock mode: /RST2; Interrupt mode: Set Mask 15
     16..31  Set P(n-16) as output, latch value, and output it
*/
void tms9901_device::write(offs_t offset, uint8_t data)
{
	write_bit(offset, data!=0);
}

void tms9901_device::write_bit(int offset, bool data)
{
	int crubit = offset & 0x001f;

	if (crubit >= 16)
	{
		LOGMASKED(LOG_PINS, "Output on P%d = %d\n", crubit-16, data);
		set_and_latch_output(crubit-P0, data);
		return;
	}

	switch (crubit)
	{
	case 0:
		// Write to control bit (CB)
		m_clock_mode = (data!=0);
		LOGMASKED(LOG_MODE, "Enter %s mode\n", m_clock_mode? "clock" : "interrupt");
		break;

	case 15:
		if (m_clock_mode)
		{
			// In clock mode, bit 15 is /RST2
			if (data == 0) soft_reset();
		}
		else
		{
			set_bit(m_int_mask, 15, data!=0);
			LOGMASKED(LOG_MASK, "/INT15 is %s\n", data? "enabled" : "disabled");
			prioritize_interrupts();     // changed interrupt state
		}
		break;

	default:
		// Bits 1..14
		if (m_clock_mode)
		{
			// Modify clock interval
			set_bit(m_clock_register, crubit-1, data!=0);

			// Reset clock timer (page 8)
			m_decrementer_value = m_clock_register;

			LOGMASKED(LOG_CLOCK, "Clock register = %04x\n", m_clock_register);
		}
		else
		{
			// Modify interrupt enable mask
			set_bit(m_int_mask, crubit, data!=0);

			// [1] sect 2.5: "When the clock interrupt is active, the clock mask
			// (mask bit 3) must be written into (with either a "0" or "1")
			// to clear the interrupt."
			if (crubit == 3)
				m_timer_int_pending = false;

			LOGMASKED(LOG_MASK, "/INT%d is %s\n", crubit, data? "enabled" : "disabled");
			prioritize_interrupts();
		}
		break;
	}
}

/*
    Update clock line. This is not a real connection to the 9901; it represents
    the effect of setting selection line S0 to 1. Since we use a separate
    I/O address space, and in the real machine, the same address lines as for
    memory access are used, and one of the address lines is connected to S0,
    we have settings of S0 even in situations when there is no I/O access but
    ordinary memory access.

    Offering a method explicitly for S0 looks inconsistent with the way of
    addressing the bits in this chip (that is, we should then offer S1 to   S4 as
    well).

    Drivers may use this line for higher emulation precision concerning the
    clock.
*/
void tms9901_device::update_clock()
{
	m_clock_read_register = m_decrementer_value;
}

/*
    decrement_tick
    Decrementer counts down the value set in clock mode; when it reaches 0,
    raises an interrupt and resets to the start value
*/
TIMER_CALLBACK_MEMBER(tms9901_device::decrement_tick)
{
	timer_clock_in(ASSERT_LINE);
	timer_clock_in(CLEAR_LINE);
}

void tms9901_device::timer_clock_in(line_state clk)
{
	if (clk == ASSERT_LINE)
	{
		m_decrementer_value = (m_decrementer_value - 1) & 0x3FFF;

		if (!m_clock_mode)
			m_clock_read_register = m_decrementer_value;

		LOGMASKED(LOG_CLOCK, "Clock = %04x\n", m_decrementer_value);
		if (m_decrementer_value==0)
		{
			if (m_clock_register != 0)
			{
				LOGMASKED(LOG_INT, "Timer expired\n");
				m_timer_int_pending = true;         // decrementer interrupt requested
				prioritize_interrupts();
			}
			m_decrementer_value = m_clock_register;
		}
	}
}

/*
    Synchronous clock input. This may be used for systems which have
    a CLK line controlled by the CPU, like the TMS99xx systems.
    In that case, clock is set to 0.
*/
void tms9901_device::phi_line(int state)
{
	if (state==ASSERT_LINE)
	{
		// Divider by 64
		m_clockdiv = (m_clockdiv+1) & 0x3f;
		if (m_clockdiv==0)
		{
			timer_clock_in(ASSERT_LINE);

			if (!m_clock_mode)
				m_clock_read_register = m_decrementer_value;

			// We signal the interrupt in sync with the clock line
			signal_int();

			// For the next phi assert
			// MZ: This costs a lot of performance for a minimum of benefit.
			if (m_poll_lines) sample_interrupt_inputs();
		}
		else
		{
			if (m_clockdiv==32)
				timer_clock_in(CLEAR_LINE);
		}
	}
}

/*
    All unmasked interrupt ports are sampled at the rising edge of phi.
    Doing it this way (also for performance issues): For each mask bit 1,
    fetch the pin level. Stop at the first asserted INT line.

    Good idea in terms of emulation, bad in terms of performance. The Geneve
    9640 bench dropped from 680% to 180%. Not recommended to use, except in
    very special situations. Enable by calling set_poll_int_lines(true).
*/
void tms9901_device::sample_interrupt_inputs()
{
	int mask = m_int_mask;
	m_int_level = 0;
	m_int_pending = false;

	while (mask != 0 && !m_int_pending)
	{
		m_int_level++;
		if ((mask & 1)!=0)
		{
			// Negative logic
			if (m_read_port(m_int_level)==0)
				m_int_pending = true;
		}
		mask >>= 1;
	}
}

void tms9901_device::soft_reset()
{
	// TMS9901 soft reset (RST2*)
	// Spec: "Writing a 0 to bit 15 while in the clock mode executes a soft reset on the I/O pins.
	// [...] RST2* will program all ports to the input mode"
	m_pio_direction = 0;
	m_pio_output = 0;

	// We assume that the interrupt mask is also reset.
	m_int_mask = 0;

	LOGMASKED(LOG_MODE, "Soft reset (RST2*)\n");
}

/*-------------------------------------------------
    device_stop - device-specific stop
-------------------------------------------------*/

void tms9901_device::device_stop()
{
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void tms9901_device::device_reset()
{
	do_reset();
}

/*
    RST1 input line (active low; using ASSERT/CLEAR).
*/
void tms9901_device::rst1_line(int state)
{
	if (state==ASSERT_LINE) do_reset();
}

void tms9901_device::do_reset()
{
	m_timer_int_pending = false;
	soft_reset();

	// This is an interrupt level latch, positive logic (bit 0 = no int)
	// The inputs are negative logic (INTx*)
	m_int_line = 0;

	prioritize_interrupts();

	m_clock_mode = false;
	m_decrementer_value = m_clock_register = 0;
}


/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void tms9901_device::device_start()
{
	// Allow for using asynchronous and synchronous clocks
	if (clock() != 0)
	{
		m_decrementer = timer_alloc(FUNC(tms9901_device::decrement_tick), this);
		m_decrementer->adjust(attotime::from_hz(clock() / 64.), 0, attotime::from_hz(clock() / 64.));
	}

	m_read_port.resolve();
	m_write_p.resolve_all_safe();
	m_interrupt.resolve();

	m_clock_register = 0;

	save_item(NAME(m_int_line));
	save_item(NAME(m_pio_output));
	save_item(NAME(m_pio_direction));
	save_item(NAME(m_int_level));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_last_level));
	save_item(NAME(m_int_pending));
	save_item(NAME(m_timer_int_pending));
	save_item(NAME(m_clock_mode));
	save_item(NAME(m_clock_register));
	save_item(NAME(m_decrementer_value));
	save_item(NAME(m_clock_read_register));
}

DEFINE_DEVICE_TYPE(TMS9901, tms9901_device, "tms9901", "TMS9901 Programmable System Interface")
