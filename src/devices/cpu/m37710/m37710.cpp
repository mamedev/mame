// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud, hap
/*
    Mitsubishi M37702/37710/37720 CPU Emulator

    The 7700 series is based on the WDC 65C816 core, with the following
    notable changes:

    - Second accumulator called "B" (on the 65816, "A" and "B" were the
      two 8-bit halves of the 16-bit "C" accumulator).
    - 6502 emulation mode and XCE instruction are not present.
    - No NMI line.  BRK and the watchdog interrupt are non-maskable, but there
      is no provision for the traditional 6502/65816 NMI line.
    - 3-bit interrupt priority levels like the 68000.  Interrupts in general
      are very different from the 65816.
    - New single-instruction immediate-to-memory move instructions (LDM)
      replaces STZ.
    - CLM and SEM (clear and set "M" status bit) replace CLD/SED.  Decimal
      mode is still available via REP/SEP instructions.
    - INC and DEC (0x1A and 0x3A) switch places for no particular reason.
    - The microcode bug that caused MVN/NVP to take 2 extra cycles per byte
      on the 65816 seems to have been fixed.
    - The WDM (0x42) and BIT immediate (0x89) instructions are now prefixes.
      0x42 when used before an instruction involving the A accumulator makes
      it use the B accumulator instead.  0x89 adds multiply and divide
      opcodes, which the real 65816 doesn't have.
    - The 65C816 preserves the upper 8 bits of A when in 8-bit M mode, but
      not the upper 8 bits of X or Y when in 8-bit X.  The 7700 preserves
      the top bits of all registers in all modes (code in the C74 BIOS
      starting at d881 requires this!).

    The various 7700 series models differ primarily by their on board
    peripherals.  The 7750 and later models do include some additional
    instructions, vs. the 770x/1x/2x, notably signed multiply/divide and
    sign extension opcodes.

    Peripherals common across the 7700 series include: programmable timers,
    digital I/O ports, and analog to digital converters.

    Reference: 7700 Family Software User's Manual (instruction set)
               7702/7703 Family User's Manual (on-board peripherals)
           7720 Family User's Manual

    Emulator by R. Belmont.
    Based on G65816 Emulator by Karl Stenrud.

    History:
    - v1.0  RB  First version, basic operation OK, timers not complete
    - v1.1  RB  Data bus is 16-bit, dozens of bugfixes to IRQs, opcodes,
                    and opcode mapping.  New opcodes added, internal timers added.
    - v1.2  RB  Fixed execution outside of bank 0, fixed LDM outside of bank 0,
                fixed so top 8 bits of X & Y are preserved while in 8-bit mode,
        added save state support.
*/

#include "emu.h"
#include "debugger.h"
#include "m37710.h"
#include "m37710cm.h"
#include "m37710il.h"

#define M37710_DEBUG    (0) // enables verbose logging for peripherals, etc.


DEFINE_DEVICE_TYPE(M37702M2, m37702m2_device, "m37702m2", "Mitsubishi M37702M2")
DEFINE_DEVICE_TYPE(M37702S1, m37702s1_device, "m37702s1", "Mitsubishi M37702S1")
DEFINE_DEVICE_TYPE(M37710S4, m37710s4_device, "m37710s4", "Mitsubishi M37710S4")
DEFINE_DEVICE_TYPE(M37720S1, m37720s1_device, "m37720s1", "Mitsubishi M37720S1")


// On-board RAM, ROM, and peripherals

// M37702M2: 512 bytes internal RAM, 16K internal mask ROM
// (M37702E2: same with EPROM instead of mask ROM)
void m37702m2_device::map(address_map &map)
{
	map(0x000000, 0x00007f).rw(FUNC(m37702m2_device::m37710_internal_r), FUNC(m37702m2_device::m37710_internal_w));
	map(0x000080, 0x00027f).ram();
	map(0x00c000, 0x00ffff).rom().region(M37710_INTERNAL_ROM_REGION, 0);
}


// M37702S1: 512 bytes internal RAM, no internal ROM
void m37702s1_device::map(address_map &map)
{
	map(0x000000, 0x00007f).rw(FUNC(m37702s1_device::m37710_internal_r), FUNC(m37702s1_device::m37710_internal_w));
	map(0x000080, 0x00027f).ram();
}


// M37710S4: 2048 bytes internal RAM, no internal ROM
void m37710s4_device::map(address_map &map)
{
	map(0x000000, 0x00007f).rw(FUNC(m37710s4_device::m37710_internal_r), FUNC(m37710s4_device::m37710_internal_w));
	map(0x000080, 0x00087f).ram();
}

// M37720S1: 512 bytes internal RAM, no internal ROM, built-in DMA
void m37720s1_device::map(address_map &map)
{
	map(0x000000, 0x00007f).rw(FUNC(m37720s1_device::m37710_internal_r), FUNC(m37720s1_device::m37710_internal_w));
	map(0x000080, 0x00027f).ram();
}

// many other combinations of RAM and ROM size exist


m37710_cpu_device::m37710_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0, map_delegate)
	, m_port_in_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_port_out_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_analog_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
{
}


m37702m2_device::m37702m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37702m2_device(mconfig, M37702M2, tag, owner, clock)
{
}


m37702m2_device::m37702m2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: m37710_cpu_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(m37702m2_device::map), this))
{
}


m37702s1_device::m37702s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37710_cpu_device(mconfig, M37702S1, tag, owner, clock, address_map_constructor(FUNC(m37702s1_device::map), this))
{
}


m37710s4_device::m37710s4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37710_cpu_device(mconfig, M37710S4, tag, owner, clock, address_map_constructor(FUNC(m37710s4_device::map), this))
{
}

m37720s1_device::m37720s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37710_cpu_device(mconfig, M37720S1, tag, owner, clock, address_map_constructor(FUNC(m37720s1_device::map), this))
{
}

std::vector<std::pair<int, const address_space_config *>> m37710_cpu_device::memory_space_config() const
{
	return std::vector<std::pair<int, const address_space_config *>> {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

/* interrupt control mapping */

const int m37710_cpu_device::m37710_irq_levels[M37710_INTERRUPT_MAX] =
{
	// maskable
	0x6f,   // DMA3          0
	0x6e,   // DMA2          1
	0x6d,   // DMA1          2
	0x6c,   // DMA0          3
	0x70,   // ADC           4
	0x73,   // UART 1 XMIT   5
	0x74,   // UART 1 RECV   6
	0x71,   // UART 0 XMIT   7
	0x72,   // UART 0 RECV   8
	0x7c,   // Timer B2      9
	0x7b,   // Timer B1     10
	0x7a,   // Timer B0     11
	0x79,   // Timer A4     12
	0x78,   // Timer A3     13
	0x77,   // Timer A2     14
	0x76,   // Timer A1     15
	0x75,   // Timer A0     16
	0x7f,   // IRQ 2        13
	0x7e,   // IRQ 1        18
	0x7d,   // IRQ 0        19

	// non-maskable
	0,  // watchdog
	0,  // debugger control
	0,  // BRK
	0,  // divide by zero
	0,  // reset
};

const int m37710_cpu_device::m37710_irq_vectors[M37710_INTERRUPT_MAX] =
{
	// maskable
	0xffce, // DMA3
	0xffd0, // DMA2
	0xffd2, // DMA1
	0xffd4, // DMA0
	0xffd6, // A-D converter
	0xffd8, // UART1 transmit
	0xffda, // UART1 receive
	0xffdc, // UART0 transmit
	0xffde, // UART0 receive
	0xffe0, // Timer B2
	0xffe2, // Timer B1
	0xffe4, // Timer B0
	0xffe6, // Timer A4
	0xffe8, // Timer A3
	0xffea, // Timer A2
	0xffec, // Timer A1
	0xffee, // Timer A0
	0xfff0, // external INT2 pin
	0xfff2, // external INT1 pin
	0xfff4, // external INT0 pin

	// non-maskable
	0xfff6, // watchdog timer
	0xfff8, // debugger control (not used in shipping ICs?)
	0xfffa, // BRK
	0xfffc, // divide by zero
	0xfffe, // RESET
};

// M37710 internal peripherals

const char *const m37710_cpu_device::m37710_rnames[128] =
{
	"",
	"",
	"Port P0 reg",
	"Port P1 reg",
	"Port P0 dir reg",
	"Port P1 dir reg",
	"Port P2 reg",
	"Port P3 reg",
	"Port P2 dir reg",
	"Port P3 dir reg",
	"Port P4 reg",      // 10 (0x0A) - ports 0, 1, 2, 3 don't exist on 37720
	"Port P5 reg",
	"Port P4 dir reg",
	"Port P5 dir reg",
	"Port P6 reg",
	"Port P7 reg",
	"Port P6 dir reg",  // 16 (0x10)
	"Port P7 dir reg",
	"Port P8 reg",
	"",
	"Port P8 dir reg",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A/D control reg",
	"A/D sweep pin select",
	"A/D 0",        // 32 (0x20)
	"",
	"A/D 1",
	"",
	"A/D 2",
	"",
	"A/D 3",
	"",
	"A/D 4",
	"",
	"A/D 5",
	"",
	"A/D 6",
	"",
	"A/D 7",
	"",
	"UART0 transmit/recv mode",     // 48 (0x30)
	"UART0 baud rate",          // 0x31
	"UART0 transmit buf L",     // 0x32
	"UART0 transmit buf H",     // 0x33
	"UART0 transmit/recv ctrl 0",   // 0x34
	"UART0 transmit/recv ctrl 1",   // 0x35
	"UART0 recv buf L",     // 0x36
	"UART0 recv buf H",     // 0x37
	"UART1 transmit/recv mode", // 0x38
	"UART1 baud rate",
	"UART1 transmit buf L",
	"UART1 transmit buf H",
	"UART1 transmit/recv ctrl 0",
	"UART1 transmit/recv ctrl 1",
	"UART1 recv buf L",
	"UART1 recv buf H",
	"Count start",          // 0x40
	"",
	"One-shot start",
	"",
	"Up-down register",
	"",
	"Timer A0 L",       // 0x46
	"Timer A0 H",
	"Timer A1 L",
	"Timer A1 H",
	"Timer A2 L",
	"Timer A2 H",
	"Timer A3 L",
	"Timer A3 H",
	"Timer A4 L",
	"Timer A4 H",
	"Timer B0 L",
	"Timer B0 H",       // 0x50
	"Timer B1 L",
	"Timer B1 H",
	"Timer B2 L",
	"Timer B2 H",
	"Timer A0 mode",
	"Timer A1 mode",
	"Timer A2 mode",
	"Timer A3 mode",
	"Timer A4 mode",
	"Timer B0 mode",
	"Timer B1 mode",
	"Timer B2 mode",
	"Processor mode",
	"",
	"Watchdog reset",       // 0x60
	"Watchdog frequency",   // 0x61
	"Real-time output control",
	"",
	"DRAM control",
	"",
	"Refresh timer",
	"",
	"DMAC control L",
	"DMAC control H",
	"",
	"",
	"DMA0 IRQ ctrl",
	"DMA1 IRQ ctrl",
	"DMA2 IRQ ctrl",
	"DMA3 IRQ ctrl",
	"A/D IRQ ctrl",
	"UART0 xmit IRQ ctrl",      // 0x70
	"UART0 recv IRQ ctrl",
	"UART1 xmit IRQ ctrl",
	"UART1 recv IRQ ctrl",
	"Timer A0 IRQ ctrl",        // 0x74
	"Timer A1 IRQ ctrl",        // 0x75
	"Timer A2 IRQ ctrl",        // 0x76
	"Timer A3 IRQ ctrl",
	"Timer A4 IRQ ctrl",        // 0x78
	"Timer B0 IRQ ctrl",
	"Timer B1 IRQ ctrl",
	"Timer B2 IRQ ctrl",
	"INT0 IRQ ctrl",
	"INT1 IRQ ctrl",
	"INT2 IRQ ctrl",
};

const char *const m37710_cpu_device::m37710_tnames[8] =
{
	"A0", "A1", "A2", "A3", "A4", "B0", "B1", "B2"
};

TIMER_CALLBACK_MEMBER( m37710_cpu_device::m37710_timer_cb )
{
	int which = param;
	int curirq = M37710_LINE_TIMERA0 - which;

//  logerror("Timer %d expired\n", which);

	m_timers[which]->adjust(m_reload[which], param);

	m37710_set_irq_line(curirq, ASSERT_LINE);
	signal_interrupt_trigger();
}

void m37710_cpu_device::m37710_external_tick(int timer, int state)
{
	// we only care if the state is "on"
	if (!state)
	{
		return;
	}

	// check if enabled and in event counter mode
	if (m_m37710_regs[0x40] & (1<<timer))
	{
		if ((m_m37710_regs[0x56+timer] & 0x3) == 1)
		{
			int upcount = 0;

			// timer b always counts down
			if (timer <= 4)
			{
				if (m_m37710_regs[0x56+timer] & 0x10)
				{
					// up/down determined by timer out pin
					upcount = m_timer_out[timer];
				}
				else
					upcount = m_m37710_regs[0x44] >> timer & 1;
			}

			int incval = (upcount) ? 1 : -1;
			int edgeval = (upcount) ? 0xff : 0x00;

			if (m_m37710_regs[0x46+(timer*2)] == edgeval)
				m_m37710_regs[0x46+(timer*2)+1] += incval;
			m_m37710_regs[0x46+(timer*2)] += incval;
		}
		else
		{
			logerror("M37710: external tick for timer %d, not in event counter mode!\n", timer);
		}
	}
}

void m37710_cpu_device::m37710_recalc_timer(int timer)
{
	int tval;
	attotime time;
	static const int tscales[4] = { 2, 16, 64, 512 };

	// check if enabled
	if (m_m37710_regs[0x40] & (1<<timer))
	{
		#if M37710_DEBUG
		logerror("Timer %d (%s) is enabled\n", timer, m37710_tnames[timer]);
		#endif

		// set the timer's value
		tval = m_m37710_regs[0x46+(timer*2)] | (m_m37710_regs[0x47+(timer*2)]<<8);

		// HACK: ignore if timer is 8MHz (MAME slows down to a crawl)
		if (tval == 0 && (m_m37710_regs[0x56+timer]&0xc0) == 0) return;

		// check timer's mode
		// modes are slightly different between timer groups A and B
		if (timer < 5)
		{
			switch (m_m37710_regs[0x56+timer] & 0x3)
			{
				case 0:         // timer mode
					time = attotime::from_hz(unscaled_clock()) * tscales[m_m37710_regs[0x56+timer]>>6];
					time *= (tval + 1);

					#if M37710_DEBUG
					logerror("Timer %d in timer mode, %f Hz\n", timer, 1.0 / time.as_double());
					#endif

					m_timers[timer]->adjust(time, timer);
					m_reload[timer] = time;
					break;

				case 1:         // event counter mode
					#if M37710_DEBUG
					logerror("Timer %d in event counter mode\n", timer);
					#endif
					break;

				case 2:     // one-shot pulse mode
					#if M37710_DEBUG
					logerror("Timer %d in one-shot mode\n", timer);
					#endif
					break;

				case 3:         // PWM mode
					#if M37710_DEBUG
					logerror("Timer %d in PWM mode\n", timer);
					#endif
					break;
			}
		}
		else
		{
			switch (m_m37710_regs[0x56+timer] & 0x3)
			{
				case 0:         // timer mode
					time = attotime::from_hz(unscaled_clock()) * tscales[m_m37710_regs[0x56+timer]>>6];
					time *= (tval + 1);

					#if M37710_DEBUG
					logerror("Timer %d in timer mode, %f Hz\n", timer, 1.0 / time.as_double());
					#endif

					m_timers[timer]->adjust(time, timer);
					m_reload[timer] = time;
					break;

				case 1:         // event counter mode
					#if M37710_DEBUG
					logerror("Timer %d in event counter mode\n", timer);
					#endif
					break;

				case 2:     // pulse period/pulse width measurement mode
					#if M37710_DEBUG
					logerror("Timer %d in pulse period/width measurement mode\n", timer);
					#endif
					break;

				case 3:
					#if M37710_DEBUG
					logerror("Timer %d in unknown mode!\n", timer);
					#endif
					break;
			}
		}
	}
}

READ8_MEMBER(m37710_cpu_device::m37710_internal_r)
{
	uint8_t d;

	#if M37710_DEBUG
	if (offset > 1)
	logerror("m37710_internal_r from %02x: %s (PC=%x)\n", (int)offset, m37710_rnames[(int)offset], REG_PB<<16 | REG_PC);
	#endif

	switch (offset)
	{
		// ports
		case 0x02: // p0
			d = m_m37710_regs[0x04];
			if (d != 0xff)
				return (m_port_in_cb[0](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x03: // p1
			d = m_m37710_regs[0x05];
			if (d != 0xff)
				return (m_port_in_cb[1](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x06: // p2
			d = m_m37710_regs[0x08];
			if (d != 0xff)
				return (m_port_in_cb[2](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x07: // p3
			d = m_m37710_regs[0x09];
			if (d != 0xff)
				return (m_port_in_cb[3](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x0a: // p4
			d = m_m37710_regs[0x0c];
			if (d != 0xff)
				return (m_port_in_cb[4](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x0b: // p5
			d = m_m37710_regs[0x0d];
			if (d != 0xff)
				return (m_port_in_cb[5](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x0e: // p6
			d = m_m37710_regs[0x10];
			if (d != 0xff)
				return (m_port_in_cb[6](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x0f: // p7
			d = m_m37710_regs[0x11];
			if (d != 0xff)
				return (m_port_in_cb[7](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;
		case 0x12: // p8
			d = m_m37710_regs[0x14];
			if (d != 0xff)
				return (m_port_in_cb[8](0,~d)&~d) | (m_m37710_regs[offset]&d);
			break;

		// A-D regs
		case 0x20:
			return m_analog_cb[0]() & 0xff;
		case 0x21:
			return m_analog_cb[0]() >> 8;
		case 0x22:
			return m_analog_cb[1]() & 0xff;
		case 0x23:
			return m_analog_cb[1]() >> 8;
		case 0x24:
			return m_analog_cb[2]() & 0xff;
		case 0x25:
			return m_analog_cb[2]() >> 8;
		case 0x26:
			return m_analog_cb[3]() & 0xff;
		case 0x27:
			return m_analog_cb[3]() >> 8;
		case 0x28:
			return m_analog_cb[4]() & 0xff;
		case 0x29:
			return m_analog_cb[4]() >> 8;
		case 0x2a:
			return m_analog_cb[5]() & 0xff;
		case 0x2b:
			return m_analog_cb[5]() >> 8;
		case 0x2c:
			return m_analog_cb[6]() & 0xff;
		case 0x2d:
			return m_analog_cb[6]() >> 8;
		case 0x2e:
			return m_analog_cb[7]() & 0xff;
		case 0x2f:
			return m_analog_cb[7]() >> 8;

		// UART control (not hooked up yet)
		case 0x34: case 0x3c:
			return 0x08;
		case 0x35: case 0x3d:
			return 0xff;

		// A-D IRQ control (also not properly hooked up yet)
		case 0x70:
			return m_m37710_regs[offset] | 8;

		default:
			return m_m37710_regs[offset];
	}

	return m_m37710_regs[offset];
}

WRITE8_MEMBER(m37710_cpu_device::m37710_internal_w)
{
	int i;
	uint8_t prevdata;
	uint8_t d;

	#if M37710_DEBUG
	if (offset != 0x60) // filter out watchdog
	logerror("m37710_internal_w %x to %02x: %s = %x\n", data, (int)offset, m37710_rnames[(int)offset], m_m37710_regs[offset]);
	#endif

	prevdata = m_m37710_regs[offset];
	m_m37710_regs[offset] = data;

	switch(offset)
	{
		// ports
		case 0x02: // p0
			d = m_m37710_regs[0x04];
			if (d != 0)
				m_port_out_cb[0](0,data&d,d);
			break;
		case 0x03: // p1
			d = m_m37710_regs[0x05];
			if (d != 0)
				m_port_out_cb[1](0,data&d,d);
			break;
		case 0x06: // p2
			d = m_m37710_regs[0x08];
			if (d != 0)
				m_port_out_cb[2](0,data&d,d);
			break;
		case 0x07: // p3
			d = m_m37710_regs[0x09];
			if (d != 0)
				m_port_out_cb[3](0,data&d,d);
			break;
		case 0x0a: // p4
			d = m_m37710_regs[0x0c];
			if (d != 0)
				m_port_out_cb[4](0,data&d,d);
			break;
		case 0x0b: // p5
			d = m_m37710_regs[0x0d];
			if (d != 0)
				m_port_out_cb[5](0,data&d,d);
			break;
		case 0x0e: // p6
			d = m_m37710_regs[0x10];
			if (d != 0)
				m_port_out_cb[6](0,data&d,d);
			break;
		case 0x0f: // p7
			d = m_m37710_regs[0x11];
			if (d != 0)
				m_port_out_cb[7](0,data&d,d);
			break;
		case 0x12: // p8
			d = m_m37710_regs[0x14];
			if (d != 0)
				m_port_out_cb[8](0,data&d,d);
			break;

		case 0x40:  // count start
			for (i = 0; i < 8; i++)
			{
				if ((data & (1<<i)) && !(prevdata & (1<<i)))
					m37710_recalc_timer(i);
			}
			break;

		// internal interrupt control
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75:
		case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c:
			m37710_set_irq_line(offset, (data & 8) ? HOLD_LINE : CLEAR_LINE);
			m37710i_update_irqs();
			break;

		// external interrupt control
		case 0x7d: case 0x7e: case 0x7f:
			m37710_set_irq_line(offset, (data & 8) ? HOLD_LINE : CLEAR_LINE);
			m37710i_update_irqs();

			// level-sense interrupts are not implemented yet
			if (data & 0x20) logerror("error M37710: INT%d level-sense\n",offset-0x7d);
			break;

		default:
			break;
	}
}

const m37710_cpu_device::opcode_func *const m37710_cpu_device::m37710i_opcodes[4] =
{
	m37710i_opcodes_M0X0,
	m37710i_opcodes_M0X1,
	m37710i_opcodes_M1X0,
	m37710i_opcodes_M1X1,
};

const m37710_cpu_device::opcode_func *const m37710_cpu_device::m37710i_opcodes2[4] =
{
	m37710i_opcodes42_M0X0,
	m37710i_opcodes42_M0X1,
	m37710i_opcodes42_M1X0,
	m37710i_opcodes42_M1X1,
};

const m37710_cpu_device::opcode_func *const m37710_cpu_device::m37710i_opcodes3[4] =
{
	m37710i_opcodes89_M0X0,
	m37710i_opcodes89_M0X1,
	m37710i_opcodes89_M1X0,
	m37710i_opcodes89_M1X1,
};

const m37710_cpu_device::get_reg_func m37710_cpu_device::m37710i_get_reg[4] =
{
	&m37710_cpu_device::m37710i_get_reg_M0X0,
	&m37710_cpu_device::m37710i_get_reg_M0X1,
	&m37710_cpu_device::m37710i_get_reg_M1X0,
	&m37710_cpu_device::m37710i_get_reg_M1X1,
};

const m37710_cpu_device::set_reg_func m37710_cpu_device::m37710i_set_reg[4] =
{
	&m37710_cpu_device::m37710i_set_reg_M0X0,
	&m37710_cpu_device::m37710i_set_reg_M0X1,
	&m37710_cpu_device::m37710i_set_reg_M1X0,
	&m37710_cpu_device::m37710i_set_reg_M1X1,
};

const m37710_cpu_device::set_line_func m37710_cpu_device::m37710i_set_line[4] =
{
	&m37710_cpu_device::m37710i_set_line_M0X0,
	&m37710_cpu_device::m37710i_set_line_M0X1,
	&m37710_cpu_device::m37710i_set_line_M1X0,
	&m37710_cpu_device::m37710i_set_line_M1X1,
};

const m37710_cpu_device::execute_func m37710_cpu_device::m37710i_execute[4] =
{
	&m37710_cpu_device::m37710i_execute_M0X0,
	&m37710_cpu_device::m37710i_execute_M0X1,
	&m37710_cpu_device::m37710i_execute_M1X0,
	&m37710_cpu_device::m37710i_execute_M1X1,
};

/* internal functions */

void m37710_cpu_device::m37710i_update_irqs()
{
	int curirq, pending = LINE_IRQ;
	int wantedIRQ = -1;
	int curpri = 0;

	for (curirq = M37710_INTERRUPT_MAX - 1; curirq >= 0; curirq--)
	{
		if ((pending & (1 << curirq)))
		{
			// this IRQ is set
			if (m37710_irq_levels[curirq])
			{
				int control = m_m37710_regs[m37710_irq_levels[curirq]];
				int thispri = control & 7;
				// logerror("line %d set, level %x curpri %x IPL %x\n", curirq, thispri, curpri, m_ipl);
				// it's maskable, check if the level works, also make sure it's acceptable for the current CPU level
				if (!FLAG_I && thispri > curpri && thispri > m_ipl)
				{
					// mark us as the best candidate
					wantedIRQ = curirq;
					curpri = thispri;
				}
			}
			else
			{
				// non-maskable
				wantedIRQ = curirq;
				curpri = 7;
				break;  // no more processing, NMIs always win
			}
		}
	}

	if (wantedIRQ != -1)
	{
		standard_irq_callback(wantedIRQ);

		// make sure we're running to service the interrupt
		CPU_STOPPED &= ~STOP_LEVEL_WAI;

		// auto-clear line
		m37710_set_irq_line(wantedIRQ, CLEAR_LINE);

		// let's do it...
		// push PB, then PC, then status
		CLK(13);
		m37710i_push_8(REG_PB>>16);
		m37710i_push_16(REG_PC);
		m37710i_push_8(m_ipl);
		m37710i_push_8(m37710i_get_reg_p());

		// set I to 1, set IPL to the interrupt we're taking
		FLAG_I = IFLAG_SET;
		m_ipl = curpri;
		// then PB=0, PC=(vector)
		REG_PB = 0;
		REG_PC = m37710_read_16(m37710_irq_vectors[wantedIRQ]);
	}
}

/* external functions */

void m37710_cpu_device::device_reset()
{
	int i;

	/* Reset MAME timers */
	for (i = 0; i < 8; i++)
	{
		m_timers[i]->reset();
		m_reload[i] = attotime::zero;
	}

	/* Start the CPU */
	CPU_STOPPED = 0;

	/* Reset internal registers */
	// port direction
	m_m37710_regs[0x04] = 0;
	m_m37710_regs[0x05] = 0;
	m_m37710_regs[0x08] = 0;
	m_m37710_regs[0x09] = 0;
	m_m37710_regs[0x0c] = 0;
	m_m37710_regs[0x0d] = 0;
	m_m37710_regs[0x10] = 0;
	m_m37710_regs[0x11] = 0;
	m_m37710_regs[0x14] = 0;

	m_m37710_regs[0x1e] &= 7; // A-D control
	m_m37710_regs[0x1f] |= 3; // A-D sweep

	// UART
	m_m37710_regs[0x30] = 0;
	m_m37710_regs[0x38] = 0;
	m_m37710_regs[0x34] = (m_m37710_regs[0x34] & 0xf0) | 8;
	m_m37710_regs[0x3c] = (m_m37710_regs[0x3c] & 0xf0) | 8;
	m_m37710_regs[0x35] = 2;
	m_m37710_regs[0x3d] = 2;
	m_m37710_regs[0x37]&= 1;
	m_m37710_regs[0x3f]&= 1;

	// timer
	m_m37710_regs[0x40] = 0;
	m_m37710_regs[0x42]&= 0x1f;
	m_m37710_regs[0x44] = 0;
	for (i = 0x56; i < 0x5e; i++)
		m_m37710_regs[i] = 0;

	m_m37710_regs[0x5e] = 0; // processor mode
	m_m37710_regs[0x61]&= 1; // watchdog frequency

	// interrupt control
	m_m37710_regs[0x7d] &= 0x3f;
	m_m37710_regs[0x7e] &= 0x3f;
	m_m37710_regs[0x7f] &= 0x3f;
	for (i = 0x70; i < 0x7d; i++)
		m_m37710_regs[i] &= 0xf;

	/* Clear IPL, m, x, D and set I */
	m_ipl = 0;
	FLAG_M = MFLAG_CLEAR;
	FLAG_X = XFLAG_CLEAR;
	FLAG_D = DFLAG_CLEAR;
	FLAG_I = IFLAG_SET;

	/* Clear all pending interrupts (should we really do this?) */
	LINE_IRQ = 0;
	IRQ_DELAY = 0;

	/* 37710 boots in full native mode */
	REG_D = 0;
	REG_PB = 0;
	REG_DB = 0;
	REG_S = (REG_S & 0xff) | 0x100;
	REG_XH = REG_X & 0xff00; REG_X &= 0xff;
	REG_YH = REG_Y & 0xff00; REG_Y &= 0xff;
	REG_B = REG_A & 0xff00; REG_A &= 0xff;
	REG_BB = REG_BA & 0xff00; REG_BA &= 0xff;

	/* Set the function tables to emulation mode */
	m37710i_set_execution_mode(EXECUTION_MODE_M0X0);

	/* Fetch the reset vector */
	REG_PC = m37710_read_16(0xfffe);
}

/* Execute some instructions */
void m37710_cpu_device::execute_run()
{
	m37710i_update_irqs();

	int clocks = m_ICount;
	m_ICount = clocks - (this->*m_execute)(m_ICount);
}


/* Set the Program Counter */
void m37710_cpu_device::m37710_set_pc(unsigned val)
{
	REG_PC = MAKE_UINT_16(val);
}

/* Get the current Stack Pointer */
unsigned m37710_cpu_device::m37710_get_sp()
{
	return REG_S;
}

/* Set the Stack Pointer */
void m37710_cpu_device::m37710_set_sp(unsigned val)
{
	REG_S = MAKE_UINT_16(val);
}

/* Get a register */
unsigned m37710_cpu_device::m37710_get_reg(int regnum)
{
	return (this->*m_get_reg)(regnum);
}

/* Set a register */
void m37710_cpu_device::m37710_set_reg(int regnum, unsigned value)
{
	(this->*m_set_reg)(regnum, value);
}

/* Set an interrupt line */
void m37710_cpu_device::m37710_set_irq_line(int line, int state)
{
	(this->*m_set_line)(line, state);
}

bool m37710_cpu_device::get_m_flag() const
{
	return FLAG_M;
}

bool m37710_cpu_device::get_x_flag() const
{
	return FLAG_X;
}

std::unique_ptr<util::disasm_interface> m37710_cpu_device::create_disassembler()
{
	return std::make_unique<m7700_disassembler>(this);
}

void m37710_cpu_device::m37710_restore_state()
{
	// restore proper function pointers
	m37710i_set_execution_mode((FLAG_M>>4) | (FLAG_X>>4));
}

void m37710_cpu_device::device_start()
{
	m_a = 0;
	m_b = 0;
	m_ba = 0;
	m_bb = 0;
	m_x = 0;
	m_y = 0;
	m_xh = 0;
	m_yh = 0;
	m_s = 0;
	m_pc = 0;
	m_ppc = 0;
	m_pb = 0;
	m_db = 0;
	m_d = 0;
	m_flag_e = 0;
	m_flag_m = 0;
	m_flag_x = 0;
	m_flag_n = 0;
	m_flag_v = 0;
	m_flag_d = 0;
	m_flag_i = 0;
	m_flag_z = 0;
	m_flag_c = 0;
	m_line_irq = 0;
	m_ipl = 0;
	m_ir = 0;
	m_im = 0;
	m_im2 = 0;
	m_im3 = 0;
	m_im4 = 0;
	m_irq_delay = 0;
	m_irq_level = 0;
	m_stopped = 0;
	memset(m_m37710_regs, 0, sizeof(m_m37710_regs));

	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<1, 0, ENDIANNESS_LITTLE>();

	for (auto &cb : m_port_in_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_port_out_cb)
		cb.resolve_safe();
	for (auto &cb : m_analog_cb)
		cb.resolve_safe(0);

	m_ICount = 0;

	m_source = 0;
	m_destination = 0;

	for (int i = 0; i < 8; i++)
	{
		m_timers[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m37710_cpu_device::m37710_timer_cb), this));
		m_reload[i] = attotime::never;
		m_timer_out[i] = 0;
	}

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_ba));
	save_item(NAME(m_bb));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_xh));
	save_item(NAME(m_yh));
	save_item(NAME(m_s));
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_pb));
	save_item(NAME(m_db));
	save_item(NAME(m_d));
	save_item(NAME(m_flag_e));
	save_item(NAME(m_flag_m));
	save_item(NAME(m_flag_x));
	save_item(NAME(m_flag_n));
	save_item(NAME(m_flag_v));
	save_item(NAME(m_flag_d));
	save_item(NAME(m_flag_i));
	save_item(NAME(m_flag_z));
	save_item(NAME(m_flag_c));
	save_item(NAME(m_line_irq));
	save_item(NAME(m_ipl));
	save_item(NAME(m_ir));
	save_item(NAME(m_im));
	save_item(NAME(m_im2));
	save_item(NAME(m_im3));
	save_item(NAME(m_im4));
	save_item(NAME(m_irq_delay));
	save_item(NAME(m_irq_level));
	save_item(NAME(m_stopped));
	save_item(NAME(m_m37710_regs));
	save_item(NAME(m_reload[0]));
	save_item(NAME(m_reload[1]));
	save_item(NAME(m_reload[2]));
	save_item(NAME(m_reload[3]));
	save_item(NAME(m_reload[4]));
	save_item(NAME(m_reload[5]));
	save_item(NAME(m_reload[6]));
	save_item(NAME(m_reload[7]));
	save_item(NAME(m_timer_out));

	machine().save().register_postload(save_prepost_delegate(save_prepost_delegate(FUNC(m37710_cpu_device::m37710_restore_state), this)));

	state_add( M37710_PC,        "PC",  m_pc).formatstr("%04X");
	state_add( M37710_PB,        "PB",  m_debugger_pb).callimport().callexport().formatstr("%02X");
	state_add( M37710_DB,        "DB",  m_debugger_db).callimport().callexport().formatstr("%02X");
	state_add( M37710_D,         "D",   m_d).formatstr("%04X");
	state_add( M37710_S,         "S",   m_s).formatstr("%04X");
	state_add( M37710_P,         "P",   m_debugger_p).callimport().callexport().formatstr("%04X");
	state_add( M37710_E,         "E",   m_flag_e).formatstr("%01X");
	state_add( M37710_A,         "A",   m_debugger_a).callimport().callexport().formatstr("%04X");
	state_add( M37710_B,         "B",   m_debugger_b).callimport().callexport().formatstr("%04X");
	state_add( M37710_X,         "X",   m_x).formatstr("%04X");
	state_add( M37710_Y,         "Y",   m_y).formatstr("%04X");
	state_add( M37710_IRQ_STATE, "IRQ", m_line_irq).formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_debugger_pc ).callimport().callexport().noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_debugger_pc ).callimport().callexport().noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_p ).formatstr("%8s").noshow();

	set_icountptr(m_ICount);
}


void m37710_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case M37710_PB:
			m37710_set_reg(M37710_PB, m_debugger_pb);
			break;

		case M37710_DB:
			m37710_set_reg(M37710_DB, m_debugger_db);
			break;

		case M37710_P:
			m37710_set_reg(M37710_P, m_debugger_p&0xff);
			m_ipl = (m_debugger_p>>8)&0xff;
			break;

		case M37710_A:
			m37710_set_reg(M37710_A, m_debugger_a);
			break;

		case M37710_B:
			m37710_set_reg(M37710_B, m_debugger_b);
			break;

		case STATE_GENPC:
		case STATE_GENPCBASE:
			REG_PB = m_debugger_pc & 0xff0000;
			m37710_set_pc(m_debugger_pc & 0xffff);
			break;
	}
}


void m37710_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case M37710_PB:
			m_debugger_pb = m_pb >> 16;
			break;

		case M37710_DB:
			m_debugger_db = m_db >> 16;
			break;

		case M37710_P:
			m_debugger_p = (m_flag_n&0x80) | ((m_flag_v>>1)&0x40) | m_flag_m | m_flag_x | m_flag_d | m_flag_i | ((!m_flag_z)<<1) | ((m_flag_c>>8)&1) | (m_ipl<<8);
			break;

		case M37710_A:
			m_debugger_a = m_a | m_b;
			break;

		case M37710_B:
			m_debugger_b = m_ba | m_bb;
			break;

		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_debugger_pc = (REG_PB | REG_PC);
			break;
	}
}


void m37710_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_flag_n & NFLAG_SET ? 'N':'.',
				m_flag_v & VFLAG_SET ? 'V':'.',
				m_flag_m & MFLAG_SET ? 'M':'.',
				m_flag_x & XFLAG_SET ? 'X':'.',
				m_flag_d & DFLAG_SET ? 'D':'.',
				m_flag_i & IFLAG_SET ? 'I':'.',
				m_flag_z == 0        ? 'Z':'.',
				m_flag_c & CFLAG_SET ? 'C':'.');
			break;
	}
}


void m37710_cpu_device::execute_set_input(int inputnum, int state)
{
	switch( inputnum )
	{
		case M37710_LINE_ADC:
		case M37710_LINE_IRQ0:
		case M37710_LINE_IRQ1:
		case M37710_LINE_IRQ2:
			m37710_set_irq_line(inputnum, state);
			break;

		case M37710_LINE_TIMERA0IN:
		case M37710_LINE_TIMERA1IN:
		case M37710_LINE_TIMERA2IN:
		case M37710_LINE_TIMERA3IN:
		case M37710_LINE_TIMERA4IN:
		case M37710_LINE_TIMERB0IN:
		case M37710_LINE_TIMERB1IN:
		case M37710_LINE_TIMERB2IN:
			m37710_external_tick(inputnum - M37710_LINE_TIMERA0IN, state);
			break;

		case M37710_LINE_TIMERA0OUT:
		case M37710_LINE_TIMERA1OUT:
		case M37710_LINE_TIMERA2OUT:
		case M37710_LINE_TIMERA3OUT:
		case M37710_LINE_TIMERA4OUT:
		case M37710_LINE_TIMERB0OUT:
		case M37710_LINE_TIMERB1OUT:
		case M37710_LINE_TIMERB2OUT:
			m_timer_out[inputnum - M37710_LINE_TIMERA0OUT] = state ? 1 : 0;
			break;
	}
}


void m37710_cpu_device::m37710i_set_execution_mode(uint32_t mode)
{
	m_opcodes = m37710i_opcodes[mode];
	m_opcodes42 = m37710i_opcodes2[mode];
	m_opcodes89 = m37710i_opcodes3[mode];
	FTABLE_GET_REG = m37710i_get_reg[mode];
	FTABLE_SET_REG = m37710i_set_reg[mode];
	FTABLE_SET_LINE = m37710i_set_line[mode];
	m_execute = m37710i_execute[mode];
}


/* ======================================================================== */
/* =============================== INTERRUPTS ============================= */
/* ======================================================================== */

void m37710_cpu_device::m37710i_interrupt_software(uint32_t vector)
{
	CLK(13);
	m37710i_push_8(REG_PB>>16);
	m37710i_push_16(REG_PC);
	m37710i_push_8(m_ipl);
	m37710i_push_8(m37710i_get_reg_p());
	FLAG_I = IFLAG_SET;
	REG_PB = 0;
	REG_PC = m37710_read_16(vector);
}



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
