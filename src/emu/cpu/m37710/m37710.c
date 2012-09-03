/*
    Mitsubishi M37702/37710 CPU Emulator

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

#define M37710_DEBUG	(0)	// enables verbose logging for peripherals, etc.

static void m37710_set_irq_line(m37710i_cpu_struct *cpustate, int line, int state);

/* interrupt control mapping */

extern const int m37710_irq_levels[M37710_LINE_MAX];
const int m37710_irq_levels[M37710_LINE_MAX] =
{
	// maskable
	0x70,	// ADC           0
	0x73,	// UART 1 XMIT   1
	0x74,	// UART 1 RECV   2
	0x71,	// UART 0 XMIT   3
	0x72,	// UART 0 RECV   4
	0x7c,	// Timer B2      5
	0x7b,	// Timer B1      6
	0x7a,	// Timer B0      7
	0x79,	// Timer A4      8
	0x78,	// Timer A3      9
	0x77,	// Timer A2      10
	0x76,	// Timer A1      11
	0x75,	// Timer A0      12
	0x7f,	// IRQ 2         13
	0x7e,	// IRQ 1         14
	0x7d,	// IRQ 0         15

	// non-maskable
	0,	// watchdog
	0,	// debugger control
	0,	// BRK
	0,	// divide by zero
	0,	// reset
};

static const int m37710_irq_vectors[M37710_LINE_MAX] =
{
	// maskable
	0xffd6, // A-D converter
	0xffd8, // UART1 transmit
	0xffda, // UART1 receive
	0xffdc, // UART0 transmit
	0xffde,	// UART0 receive
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
	0xfff8,	// debugger control (not used in shipping ICs?)
	0xfffa, // BRK
	0xfffc, // divide by zero
	0xfffe,	// RESET
};

// M37710 internal peripherals

#if M37710_DEBUG
static const char *const m37710_rnames[128] =
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
	"Port P4 reg",
	"Port P5 reg",
	"Port P4 dir reg",
	"Port P5 dir reg",
	"Port P6 reg",
	"Port P7 reg",
	"Port P6 dir reg",	// 16 (0x10)
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
	"A/D 0",		// 32 (0x20)
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
	"UART0 transmit/recv mode", 	// 48 (0x30)
	"UART0 baud rate",	    	// 0x31
	"UART0 transmit buf L",		// 0x32
	"UART0 transmit buf H",		// 0x33
	"UART0 transmit/recv ctrl 0",	// 0x34
	"UART0 transmit/recv ctrl 1",	// 0x35
	"UART0 recv buf L",		// 0x36
	"UART0 recv buf H",		// 0x37
	"UART1 transmit/recv mode",	// 0x38
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
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
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

static const char *const m37710_tnames[8] =
{
	"A0", "A1", "A2", "A3", "A4", "B0", "B1", "B2"
};
#endif

static TIMER_CALLBACK( m37710_timer_cb )
{
	m37710i_cpu_struct *cpustate = (m37710i_cpu_struct *)ptr;
	int which = param;
	int curirq = M37710_LINE_TIMERA0 - which;

	cpustate->timers[which]->adjust(cpustate->reload[which], param);

	m37710_set_irq_line(cpustate, curirq, HOLD_LINE);
	device_triggerint(cpustate->device);
}

static void m37710_external_tick(m37710i_cpu_struct *cpustate, int timer, int state)
{
	// we only care if the state is "on"
	if (!state)
	{
		return;
	}

	// check if enabled
	if (cpustate->m37710_regs[0x40] & (1<<timer))
	{
		if ((cpustate->m37710_regs[0x56+timer] & 0x3) == 1)
		{
			if (cpustate->m37710_regs[0x46+(timer*2)] == 0xff)
			{
				cpustate->m37710_regs[0x46+(timer*2)] = 0;
				cpustate->m37710_regs[0x46+(timer*2)+1]++;
			}
			else
			{
				cpustate->m37710_regs[0x46+(timer*2)]++;
			}
		}
		else
		{
			logerror("M37710: external tick for timer %d, not in event counter mode!\n", timer);
		}
	}
}

static void m37710_recalc_timer(m37710i_cpu_struct *cpustate, int timer)
{
	int tval;
	attotime time;
	static const int tscales[4] = { 2, 16, 64, 512 };

	// check if enabled
	if (cpustate->m37710_regs[0x40] & (1<<timer))
	{
		#if M37710_DEBUG
		logerror("Timer %d (%s) is enabled\n", timer, m37710_tnames[timer]);
		#endif

		// set the timer's value
		tval = cpustate->m37710_regs[0x46+(timer*2)] | (cpustate->m37710_regs[0x47+(timer*2)]<<8);

		// HACK: ignore if timer is 8MHz (MAME slows down to a crawl)
		if (tval == 0 && (cpustate->m37710_regs[0x56+timer]&0xc0) == 0) return;

		// check timer's mode
		// modes are slightly different between timer groups A and B
		if (timer < 5)
		{
			switch (cpustate->m37710_regs[0x56+timer] & 0x3)
			{
				case 0:	    	// timer mode
					time = attotime::from_hz(cpustate->device->unscaled_clock()) * tscales[cpustate->m37710_regs[0x56+timer]>>6];
					time *= (tval + 1);

					#if M37710_DEBUG
					logerror("Timer %d in timer mode, %f Hz\n", timer, 1.0 / time.as_double());
					#endif

					cpustate->timers[timer]->adjust(time, timer);
					cpustate->reload[timer] = time;
					break;

				case 1:	    	// event counter mode
					#if M37710_DEBUG
					logerror("Timer %d in event counter mode\n", timer);
					#endif
					break;

				case 2:		// one-shot pulse mode
					#if M37710_DEBUG
					logerror("Timer %d in one-shot mode\n", timer);
					#endif
					break;

				case 3:	    	// PWM mode
					#if M37710_DEBUG
					logerror("Timer %d in PWM mode\n", timer);
					#endif
					break;
			}
		}
		else
		{
			switch (cpustate->m37710_regs[0x56+timer] & 0x3)
			{
				case 0:	    	// timer mode
					time = attotime::from_hz(cpustate->device->unscaled_clock()) * tscales[cpustate->m37710_regs[0x56+timer]>>6];
					time *= (tval + 1);

					#if M37710_DEBUG
					logerror("Timer %d in timer mode, %f Hz\n", timer, 1.0 / time.as_double());
					#endif

					cpustate->timers[timer]->adjust(time, timer);
					cpustate->reload[timer] = time;
					break;

				case 1:	    	// event counter mode
					#if M37710_DEBUG
					logerror("Timer %d in event counter mode\n", timer);
					#endif
					break;

				case 2:		// pulse period/pulse width measurement mode
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

static UINT8 m37710_internal_r(m37710i_cpu_struct *cpustate, int offset)
{
	UINT8 d;

	#if M37710_DEBUG
	if (offset > 1)
	logerror("m37710_internal_r from %02x: %s (PC=%x)\n", (int)offset, m37710_rnames[(int)offset], REG_PB<<16 | REG_PC);
	#endif

	switch (offset)
	{
		// ports
		case 0x02: // p0
			d = cpustate->m37710_regs[0x04];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT0)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x03: // p1
			d = cpustate->m37710_regs[0x05];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT1)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x06: // p2
			d = cpustate->m37710_regs[0x08];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT2)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x07: // p3
			d = cpustate->m37710_regs[0x09];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT3)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x0a: // p4
			d = cpustate->m37710_regs[0x0c];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT4)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x0b: // p5
			d = cpustate->m37710_regs[0x0d];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT5)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x0e: // p6
			d = cpustate->m37710_regs[0x10];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT6)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x0f: // p7
			d = cpustate->m37710_regs[0x11];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT7)&~d) | (cpustate->m37710_regs[offset]&d);
			break;
		case 0x12: // p8
			d = cpustate->m37710_regs[0x14];
			if (d != 0xff)
				return (cpustate->io->read_byte(M37710_PORT8)&~d) | (cpustate->m37710_regs[offset]&d);
			break;

		// A-D regs
		case 0x20:
			return cpustate->io->read_byte(M37710_ADC0_L);
		case 0x21:
			return cpustate->io->read_byte(M37710_ADC0_H);
		case 0x22:
			return cpustate->io->read_byte(M37710_ADC1_L);
		case 0x23:
			return cpustate->io->read_byte(M37710_ADC1_H);
		case 0x24:
			return cpustate->io->read_byte(M37710_ADC2_L);
		case 0x25:
			return cpustate->io->read_byte(M37710_ADC2_H);
		case 0x26:
			return cpustate->io->read_byte(M37710_ADC3_L);
		case 0x27:
			return cpustate->io->read_byte(M37710_ADC3_H);
		case 0x28:
			return cpustate->io->read_byte(M37710_ADC4_L);
		case 0x29:
			return cpustate->io->read_byte(M37710_ADC4_H);
		case 0x2a:
			return cpustate->io->read_byte(M37710_ADC5_L);
		case 0x2b:
			return cpustate->io->read_byte(M37710_ADC5_H);
		case 0x2c:
			return cpustate->io->read_byte(M37710_ADC6_L);
		case 0x2d:
			return cpustate->io->read_byte(M37710_ADC6_H);
		case 0x2e:
			return cpustate->io->read_byte(M37710_ADC7_L);
		case 0x2f:
			return cpustate->io->read_byte(M37710_ADC7_H);

		// UART control (not hooked up yet)
		case 0x34: case 0x3c:
			return 0x08;
		case 0x35: case 0x3d:
			return 0xff;

		// A-D IRQ control (also not properly hooked up yet)
		case 0x70:
			return cpustate->m37710_regs[offset] | 8;

		default:
			return cpustate->m37710_regs[offset];
	}

	return cpustate->m37710_regs[offset];
}

static void m37710_internal_w(m37710i_cpu_struct *cpustate, int offset, UINT8 data)
{
	int i;
	UINT8 prevdata;
	UINT8 d;

	#if M37710_DEBUG
	if (offset != 0x60)	// filter out watchdog
	logerror("m37710_internal_w %x to %02x: %s = %x\n", data, (int)offset, m37710_rnames[(int)offset], cpustate->m37710_regs[offset]);
	#endif

	prevdata = cpustate->m37710_regs[offset];
	cpustate->m37710_regs[offset] = data;

	switch(offset)
	{
		// ports
		case 0x02: // p0
			d = cpustate->m37710_regs[0x04];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT0, data&d);
			break;
		case 0x03: // p1
			d = cpustate->m37710_regs[0x05];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT1, data&d);
			break;
		case 0x06: // p2
			d = cpustate->m37710_regs[0x08];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT2, data&d);
			break;
		case 0x07: // p3
			d = cpustate->m37710_regs[0x09];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT3, data&d);
			break;
		case 0x0a: // p4
			d = cpustate->m37710_regs[0x0c];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT4, data&d);
			break;
		case 0x0b: // p5
			d = cpustate->m37710_regs[0x0d];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT5, data&d);
			break;
		case 0x0e: // p6
			d = cpustate->m37710_regs[0x10];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT6, data&d);
			break;
		case 0x0f: // p7
			d = cpustate->m37710_regs[0x11];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT7, data&d);
			break;
		case 0x12: // p8
			d = cpustate->m37710_regs[0x14];
			if (d != 0)
				cpustate->io->write_byte(M37710_PORT8, data&d);
			break;

		case 0x40:	// count start
			for (i = 0; i < 8; i++)
			{
				if ((data & (1<<i)) && !(prevdata & (1<<i)))
					m37710_recalc_timer(cpustate, i);
			}
			break;

		// internal interrupt control
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75:
		case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c:
			m37710_set_irq_line(cpustate, offset, (data & 8) ? HOLD_LINE : CLEAR_LINE);
			m37710i_update_irqs(cpustate);
			break;

		// external interrupt control
		case 0x7d: case 0x7e: case 0x7f:
			m37710_set_irq_line(cpustate, offset, (data & 8) ? HOLD_LINE : CLEAR_LINE);
			m37710i_update_irqs(cpustate);

			// level-sense interrupts are not implemented yet
			if (data & 0x20) logerror("error M37710: INT%d level-sense\n",offset-0x7d);
			break;

		default:
			break;
	}
}

static READ16_HANDLER( m37710_internal_word_r )
{
	m37710i_cpu_struct *cpustate = get_safe_token(&space->device());
	UINT16 ret = 0;

	if (mem_mask & 0x00ff)
		ret |= m37710_internal_r(cpustate, offset*2);
	if (mem_mask & 0xff00)
		ret |= m37710_internal_r(cpustate, offset*2+1)<<8;

	return ret;
}

static WRITE16_HANDLER( m37710_internal_word_w )
{
	m37710i_cpu_struct *cpustate = get_safe_token(&space->device());

	if (mem_mask & 0x00ff)
		m37710_internal_w(cpustate, offset*2, data & 0xff);
	if (mem_mask & 0xff00)
		m37710_internal_w(cpustate, offset*2+1, data>>8);
}

extern void (*const m37710i_opcodes_M0X0[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes42_M0X0[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes89_M0X0[])(m37710i_cpu_struct *cpustate);
extern uint m37710i_get_reg_M0X0(m37710i_cpu_struct *cpustate, int regnum);
extern void m37710i_set_reg_M0X0(m37710i_cpu_struct *cpustate, int regnum, uint val);
extern void m37710i_set_line_M0X0(m37710i_cpu_struct *cpustate, int line, int state);
extern int  m37710i_execute_M0X0(m37710i_cpu_struct *cpustate, int cycles);

extern void (*const m37710i_opcodes_M0X1[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes42_M0X1[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes89_M0X1[])(m37710i_cpu_struct *cpustate);
extern uint m37710i_get_reg_M0X1(m37710i_cpu_struct *cpustate, int regnum);
extern void m37710i_set_reg_M0X1(m37710i_cpu_struct *cpustate, int regnum, uint val);
extern void m37710i_set_line_M0X1(m37710i_cpu_struct *cpustate, int line, int state);
extern int  m37710i_execute_M0X1(m37710i_cpu_struct *cpustate, int cycles);

extern void (*const m37710i_opcodes_M1X0[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes42_M1X0[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes89_M1X0[])(m37710i_cpu_struct *cpustate);
extern uint m37710i_get_reg_M1X0(m37710i_cpu_struct *cpustate, int regnum);
extern void m37710i_set_reg_M1X0(m37710i_cpu_struct *cpustate, int regnum, uint val);
extern void m37710i_set_line_M1X0(m37710i_cpu_struct *cpustate, int line, int state);
extern int  m37710i_execute_M1X0(m37710i_cpu_struct *cpustate, int cycles);

extern void (*const m37710i_opcodes_M1X1[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes42_M1X1[])(m37710i_cpu_struct *cpustate);
extern void (*const m37710i_opcodes89_M1X1[])(m37710i_cpu_struct *cpustate);
extern uint m37710i_get_reg_M1X1(m37710i_cpu_struct *cpustate, int regnum);
extern void m37710i_set_reg_M1X1(m37710i_cpu_struct *cpustate, int regnum, uint val);
extern void m37710i_set_line_M1X1(m37710i_cpu_struct *cpustate, int line, int state);
extern int  m37710i_execute_M1X1(m37710i_cpu_struct *cpustate, int cycles);

void (*const *const m37710i_opcodes[4])(m37710i_cpu_struct *cpustate) =
{
	m37710i_opcodes_M0X0,
	m37710i_opcodes_M0X1,
	m37710i_opcodes_M1X0,
	m37710i_opcodes_M1X1,
};

void (*const *const m37710i_opcodes2[4])(m37710i_cpu_struct *cpustate) =
{
	m37710i_opcodes42_M0X0,
	m37710i_opcodes42_M0X1,
	m37710i_opcodes42_M1X0,
	m37710i_opcodes42_M1X1,
};

void (*const *const m37710i_opcodes3[4])(m37710i_cpu_struct *cpustate) =
{
	m37710i_opcodes89_M0X0,
	m37710i_opcodes89_M0X1,
	m37710i_opcodes89_M1X0,
	m37710i_opcodes89_M1X1,
};

uint (*const m37710i_get_reg[4])(m37710i_cpu_struct *cpustate, int regnum) =
{
	m37710i_get_reg_M0X0,
	m37710i_get_reg_M0X1,
	m37710i_get_reg_M1X0,
	m37710i_get_reg_M1X1,
};

void (*const m37710i_set_reg[4])(m37710i_cpu_struct *cpustate, int regnum, uint val) =
{
	m37710i_set_reg_M0X0,
	m37710i_set_reg_M0X1,
	m37710i_set_reg_M1X0,
	m37710i_set_reg_M1X1,
};

void (*const m37710i_set_line[4])(m37710i_cpu_struct *cpustate, int line, int state) =
{
	m37710i_set_line_M0X0,
	m37710i_set_line_M0X1,
	m37710i_set_line_M1X0,
	m37710i_set_line_M1X1,
};

int (*const m37710i_execute[4])(m37710i_cpu_struct *cpustate, int cycles) =
{
	m37710i_execute_M0X0,
	m37710i_execute_M0X1,
	m37710i_execute_M1X0,
	m37710i_execute_M1X1,
};

/* internal functions */

INLINE void m37710i_push_8(m37710i_cpu_struct *cpustate, uint value)
{
	m37710_write_8(REG_S, value);
	REG_S = MAKE_UINT_16(REG_S-1);
}

INLINE void m37710i_push_16(m37710i_cpu_struct *cpustate, uint value)
{
	m37710i_push_8(cpustate, value>>8);
	m37710i_push_8(cpustate, value&0xff);
}

INLINE uint m37710i_get_reg_p(m37710i_cpu_struct *cpustate)
{
	return	(FLAG_N&0x80)		|
			((FLAG_V>>1)&0x40)	|
			FLAG_M				|
			FLAG_X				|
			FLAG_D				|
			FLAG_I				|
			((!FLAG_Z)<<1)		|
			((FLAG_C>>8)&1);
}

void m37710i_update_irqs(m37710i_cpu_struct *cpustate)
{
	int curirq, pending = LINE_IRQ;
	int wantedIRQ = -1;
	int curpri = 0;

	for (curirq = M37710_LINE_MAX - 1; curirq >= 0; curirq--)
	{
		if ((pending & (1 << curirq)))
		{
			// this IRQ is set
			if (m37710_irq_levels[curirq])
			{
				int control = cpustate->m37710_regs[m37710_irq_levels[curirq]];
				int thispri = control & 7;
				// logerror("line %d set, level %x curpri %x IPL %x\n", curirq, thispri, curpri, cpustate->ipl);
				// it's maskable, check if the level works, also make sure it's acceptable for the current CPU level
				if (!FLAG_I && thispri > curpri && thispri > cpustate->ipl)
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
				break;	// no more processing, NMIs always win
			}
		}
	}

	if (wantedIRQ != -1)
	{
		if (INT_ACK) INT_ACK(cpustate->device, wantedIRQ);

		// make sure we're running to service the interrupt
		CPU_STOPPED &= ~STOP_LEVEL_WAI;

		// auto-clear line
		m37710_set_irq_line(cpustate, wantedIRQ, CLEAR_LINE);

		// let's do it...
		// push PB, then PC, then status
		CLK(13);
//      mame_printf_debug("taking IRQ %d: PC = %06x, SP = %04x, IPL %d\n", wantedIRQ, REG_PB | REG_PC, REG_S, cpustate->ipl);
		m37710i_push_8(cpustate, REG_PB>>16);
		m37710i_push_16(cpustate, REG_PC);
		m37710i_push_8(cpustate, cpustate->ipl);
		m37710i_push_8(cpustate, m37710i_get_reg_p(cpustate));

		// set I to 1, set IPL to the interrupt we're taking
		FLAG_I = IFLAG_SET;
		cpustate->ipl = curpri;
		// then PB=0, PC=(vector)
		REG_PB = 0;
		REG_PC = m37710_read_8(m37710_irq_vectors[wantedIRQ]) |
			 m37710_read_8(m37710_irq_vectors[wantedIRQ]+1)<<8;
//      logerror("IRQ @ %06x\n", REG_PB | REG_PC);
		m37710i_jumping(REG_PB | REG_PC);
	}
}

/* external functions */

static CPU_RESET( m37710 )
{
	int i;
	m37710i_cpu_struct *cpustate = get_safe_token(device);

	/* Reset MAME timers */
	for (i = 0; i < 8; i++)
	{
		cpustate->timers[i]->reset();
		cpustate->reload[i] = attotime::zero;
	}

	/* Start the CPU */
	CPU_STOPPED = 0;

	/* Reset internal registers */
	// port direction
	cpustate->m37710_regs[0x04] = 0;
	cpustate->m37710_regs[0x05] = 0;
	cpustate->m37710_regs[0x08] = 0;
	cpustate->m37710_regs[0x09] = 0;
	cpustate->m37710_regs[0x0c] = 0;
	cpustate->m37710_regs[0x0d] = 0;
	cpustate->m37710_regs[0x10] = 0;
	cpustate->m37710_regs[0x11] = 0;
	cpustate->m37710_regs[0x14] = 0;

	cpustate->m37710_regs[0x1e] &= 7; // A-D control
	cpustate->m37710_regs[0x1f] |= 3; // A-D sweep

	// UART
	cpustate->m37710_regs[0x30] = 0;
	cpustate->m37710_regs[0x38] = 0;
	cpustate->m37710_regs[0x34] = (cpustate->m37710_regs[0x34] & 0xf0) | 8;
	cpustate->m37710_regs[0x3c] = (cpustate->m37710_regs[0x3c] & 0xf0) | 8;
	cpustate->m37710_regs[0x35] = 2;
	cpustate->m37710_regs[0x3d] = 2;
	cpustate->m37710_regs[0x37]&= 1;
	cpustate->m37710_regs[0x3f]&= 1;

	// timer
	cpustate->m37710_regs[0x40] = 0;
	cpustate->m37710_regs[0x42]&= 0x1f;
	cpustate->m37710_regs[0x44] = 0;
	for (i = 0x56; i < 0x5e; i++)
		cpustate->m37710_regs[i] = 0;

	cpustate->m37710_regs[0x5e] = 0; // processor mode
	cpustate->m37710_regs[0x61]&= 1; // watchdog frequency

	// interrupt control
	cpustate->m37710_regs[0x7d] &= 0x3f;
	cpustate->m37710_regs[0x7e] &= 0x3f;
	cpustate->m37710_regs[0x7f] &= 0x3f;
	for (i = 0x70; i < 0x7d; i++)
		cpustate->m37710_regs[i] &= 0xf;

	/* Clear IPL, m, x, D and set I */
	cpustate->ipl = 0;
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
	m37710i_set_execution_mode(cpustate, EXECUTION_MODE_M0X0);

	/* Fetch the reset vector */
	REG_PC = m37710_read_8(0xfffe) | (m37710_read_8(0xffff)<<8);
	m37710i_jumping(REG_PB | REG_PC);
}

/* Exit and clean up */
static CPU_EXIT( m37710 )
{
	/* nothing to do yet */
}

/* Execute some instructions */
static CPU_EXECUTE( m37710 )
{
	m37710i_cpu_struct *m37710 = get_safe_token(device);

	m37710i_update_irqs(m37710);

	int clocks = m37710->ICount;
	m37710->ICount = clocks - m37710->execute(m37710, m37710->ICount);
}


/* Set the Program Counter */
static void m37710_set_pc(m37710i_cpu_struct *cpustate, unsigned val)
{
	REG_PC = MAKE_UINT_16(val);
	m37710_jumping(REG_PB | REG_PC);
}

/* Get the current Stack Pointer */
static unsigned m37710_get_sp(m37710i_cpu_struct *cpustate)
{
	return REG_S;
}

/* Set the Stack Pointer */
static void m37710_set_sp(m37710i_cpu_struct *cpustate, unsigned val)
{
	REG_S = MAKE_UINT_16(val);
}

/* Get a register */
static unsigned m37710_get_reg(m37710i_cpu_struct *cpustate, int regnum)
{
	return FTABLE_GET_REG(cpustate, regnum);
}

/* Set a register */
static void m37710_set_reg(m37710i_cpu_struct *cpustate, int regnum, unsigned value)
{
	FTABLE_SET_REG(cpustate, regnum, value);
}

/* Set an interrupt line */
static void m37710_set_irq_line(m37710i_cpu_struct *cpustate, int line, int state)
{
	FTABLE_SET_LINE(cpustate, line, state);
}

/* Set the callback that is called when servicing an interrupt */
#ifdef UNUSED_FUNCTION
void m37710_set_irq_callback(device_irq_acknowledge_callback callback)
{
	INT_ACK = callback;
}
#endif

/* Disassemble an instruction */
#include "m7700ds.h"

static CPU_DISASSEMBLE( m37710 )
{
	m37710i_cpu_struct *cpustate = get_safe_token(device);

	return m7700_disassemble(buffer, (pc&0xffff), pc>>16, oprom, FLAG_M, FLAG_X);
}

static void m37710_restore_state(m37710i_cpu_struct *cpustate)
{
	// restore proper function pointers
	m37710i_set_execution_mode(cpustate, (FLAG_M>>4) | (FLAG_X>>4));

	// make sure the memory system can keep up
	m37710i_jumping(REG_PB | REG_PC);
}

static CPU_INIT( m37710 )
{
	m37710i_cpu_struct *cpustate = get_safe_token(device);
	int i;

	memset(cpustate, 0, sizeof(*cpustate));

	INT_ACK = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->ICount = 0;

	cpustate->source = 0;
	cpustate->destination = 0;

	for (i = 0; i < 8; i++)
		cpustate->timers[i] = device->machine().scheduler().timer_alloc(FUNC(m37710_timer_cb), cpustate);

	device->save_item(NAME(cpustate->a));
	device->save_item(NAME(cpustate->b));
	device->save_item(NAME(cpustate->ba));
	device->save_item(NAME(cpustate->bb));
	device->save_item(NAME(cpustate->x));
	device->save_item(NAME(cpustate->y));
	device->save_item(NAME(cpustate->xh));
	device->save_item(NAME(cpustate->yh));
	device->save_item(NAME(cpustate->s));
	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->ppc));
	device->save_item(NAME(cpustate->pb));
	device->save_item(NAME(cpustate->db));
	device->save_item(NAME(cpustate->d));
	device->save_item(NAME(cpustate->flag_e));
	device->save_item(NAME(cpustate->flag_m));
	device->save_item(NAME(cpustate->flag_x));
	device->save_item(NAME(cpustate->flag_n));
	device->save_item(NAME(cpustate->flag_v));
	device->save_item(NAME(cpustate->flag_d));
	device->save_item(NAME(cpustate->flag_i));
	device->save_item(NAME(cpustate->flag_z));
	device->save_item(NAME(cpustate->flag_c));
	device->save_item(NAME(cpustate->line_irq));
	device->save_item(NAME(cpustate->ipl));
	device->save_item(NAME(cpustate->ir));
	device->save_item(NAME(cpustate->im));
	device->save_item(NAME(cpustate->im2));
	device->save_item(NAME(cpustate->im3));
	device->save_item(NAME(cpustate->im4));
	device->save_item(NAME(cpustate->irq_delay));
	device->save_item(NAME(cpustate->irq_level));
	device->save_item(NAME(cpustate->stopped));
	device->save_item(NAME(cpustate->m37710_regs));
	device->save_item(NAME(cpustate->reload[0]));
	device->save_item(NAME(cpustate->reload[1]));
	device->save_item(NAME(cpustate->reload[2]));
	device->save_item(NAME(cpustate->reload[3]));
	device->save_item(NAME(cpustate->reload[4]));
	device->save_item(NAME(cpustate->reload[5]));
	device->save_item(NAME(cpustate->reload[6]));
	device->save_item(NAME(cpustate->reload[7]));

	device->machine().save().register_postload(save_prepost_delegate(FUNC(m37710_restore_state), cpustate));
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m37710 )
{
	m37710i_cpu_struct *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_ADC: 	m37710_set_irq_line(cpustate, M37710_LINE_ADC, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_IRQ0:	m37710_set_irq_line(cpustate, M37710_LINE_IRQ0, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_IRQ1:	m37710_set_irq_line(cpustate, M37710_LINE_IRQ1, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_IRQ2:	m37710_set_irq_line(cpustate, M37710_LINE_IRQ2, info->i); break;

		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERA0TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERA1TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERA2TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERA3TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERA4TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERB0TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERB1TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_TIMERB2TICK: m37710_external_tick(cpustate, state - CPUINFO_INT_INPUT_STATE - M37710_LINE_TIMERA0TICK, info->i); break;

		case CPUINFO_INT_PC:							REG_PB = info->i & 0xff0000; m37710_set_pc(cpustate, info->i & 0xffff); break;
		case CPUINFO_INT_SP:							m37710_set_sp(cpustate, info->i);	    			break;

		case CPUINFO_INT_REGISTER + M37710_PC:			m37710_set_reg(cpustate, M37710_PC, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_S:			m37710_set_reg(cpustate, M37710_S, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_P:			m37710_set_reg(cpustate, M37710_P, info->i&0xff);	cpustate->ipl = (info->i>>8)&0xff;	break;
		case CPUINFO_INT_REGISTER + M37710_A:			m37710_set_reg(cpustate, M37710_A, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_X:			m37710_set_reg(cpustate, M37710_X, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_Y:			m37710_set_reg(cpustate, M37710_Y, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_PB:			m37710_set_reg(cpustate, M37710_PB, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_DB:			m37710_set_reg(cpustate, M37710_DB, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_D:			m37710_set_reg(cpustate, M37710_D, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_E:			m37710_set_reg(cpustate, M37710_E, info->i);		break;
		case CPUINFO_INT_REGISTER + M37710_NMI_STATE:	m37710_set_reg(cpustate, M37710_NMI_STATE, info->i); break;
		case CPUINFO_INT_REGISTER + M37710_IRQ_STATE:	m37710_set_reg(cpustate, M37710_IRQ_STATE, info->i); break;
	}
}

// On-board RAM and peripherals
static ADDRESS_MAP_START( m37710_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000000, 0x00007f) AM_READWRITE_LEGACY(m37710_internal_word_r, m37710_internal_word_w)
	AM_RANGE(0x000080, 0x00027f) AM_RAM
ADDRESS_MAP_END

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m37710 )
{
	m37710i_cpu_struct *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m37710i_cpu_struct);			break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20; /* rough guess */			break;
		case CPUINFO_INT_INPUT_LINES:       			info->i = M37710_LINE_MAX;				break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M37710_LINE_IRQ0:	info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_IRQ1:	info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_IRQ2:	info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + M37710_LINE_RESET:	info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = REG_PPC;						break;
		case CPUINFO_INT_PC:							info->i = (REG_PB | REG_PC);			break;
		case CPUINFO_INT_SP:							info->i = m37710_get_sp(cpustate);				break;

		case CPUINFO_INT_REGISTER + M37710_PC:			info->i = m37710_get_reg(cpustate, M37710_PC);	break;
		case CPUINFO_INT_REGISTER + M37710_S:			info->i = m37710_get_reg(cpustate, M37710_S);		break;
		case CPUINFO_INT_REGISTER + M37710_P:			info->i = m37710_get_reg(cpustate, M37710_P) | (cpustate->ipl<<8); break;
		case CPUINFO_INT_REGISTER + M37710_A:			info->i = m37710_get_reg(cpustate, M37710_A);		break;
		case CPUINFO_INT_REGISTER + M37710_B:			info->i = m37710_get_reg(cpustate, M37710_B);		break;
		case CPUINFO_INT_REGISTER + M37710_X:			info->i = m37710_get_reg(cpustate, M37710_X);		break;
		case CPUINFO_INT_REGISTER + M37710_Y:			info->i = m37710_get_reg(cpustate, M37710_Y);		break;
		case CPUINFO_INT_REGISTER + M37710_PB:			info->i = m37710_get_reg(cpustate, M37710_PB);	break;
		case CPUINFO_INT_REGISTER + M37710_DB:			info->i = m37710_get_reg(cpustate, M37710_DB);	break;
		case CPUINFO_INT_REGISTER + M37710_D:			info->i = m37710_get_reg(cpustate, M37710_D);		break;
		case CPUINFO_INT_REGISTER + M37710_E:			info->i = m37710_get_reg(cpustate, M37710_E);		break;
		case CPUINFO_INT_REGISTER + M37710_NMI_STATE:	info->i = m37710_get_reg(cpustate, M37710_NMI_STATE); break;
		case CPUINFO_INT_REGISTER + M37710_IRQ_STATE:	info->i = m37710_get_reg(cpustate, M37710_IRQ_STATE); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m37710);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m37710);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m37710);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m37710);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m37710);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m37710);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->ICount;			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(m37710_internal_map); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map16 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map8 = NULL;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M37710");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "M7700");				break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright R. Belmont, based on G65816 by Karl Stenerud"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->flag_n & NFLAG_SET ? 'N':'.',
				cpustate->flag_v & VFLAG_SET ? 'V':'.',
				cpustate->flag_m & MFLAG_SET ? 'M':'.',
				cpustate->flag_x & XFLAG_SET ? 'X':'.',
				cpustate->flag_d & DFLAG_SET ? 'D':'.',
				cpustate->flag_i & IFLAG_SET ? 'I':'.',
				cpustate->flag_z == 0        ? 'Z':'.',
				cpustate->flag_c & CFLAG_SET ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M37710_PC:			sprintf(info->s, "PC:%04X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + M37710_PB:			sprintf(info->s, "PB:%02X", cpustate->pb>>16); break;
		case CPUINFO_STR_REGISTER + M37710_DB:			sprintf(info->s, "DB:%02X", cpustate->db>>16); break;
		case CPUINFO_STR_REGISTER + M37710_D:			sprintf(info->s, "D:%04X", cpustate->d); break;
		case CPUINFO_STR_REGISTER + M37710_S:			sprintf(info->s, "S:%04X", cpustate->s); break;
		case CPUINFO_STR_REGISTER + M37710_P:			sprintf(info->s, "P:%04X",
																 (cpustate->flag_n&0x80)		|
																((cpustate->flag_v>>1)&0x40)	|
																cpustate->flag_m				|
																cpustate->flag_x				|
																cpustate->flag_d				|
																cpustate->flag_i				|
																((!cpustate->flag_z)<<1)		|
																((cpustate->flag_c>>8)&1) | (cpustate->ipl<<8)); break;
		case CPUINFO_STR_REGISTER + M37710_E:			sprintf(info->s, "E:%d", cpustate->flag_e); break;
		case CPUINFO_STR_REGISTER + M37710_A:			sprintf(info->s, "A:%04X", cpustate->a | cpustate->b); break;
		case CPUINFO_STR_REGISTER + M37710_B:			sprintf(info->s, "B:%04X", cpustate->ba | cpustate->bb); break;
		case CPUINFO_STR_REGISTER + M37710_X:			sprintf(info->s, "X:%04X", cpustate->x); break;
		case CPUINFO_STR_REGISTER + M37710_Y:			sprintf(info->s, "Y:%04X", cpustate->y); break;
		case CPUINFO_STR_REGISTER + M37710_IRQ_STATE:	sprintf(info->s, "IRQ:%X", cpustate->line_irq); break;
	}
}

// 37702 is identical except with an internal ROM, so just change the name
CPU_GET_INFO( m37702 )
{
	if (state == CPUINFO_STR_NAME)
	{
		strcpy(info->s, "M37702");
		return;
	}

	CPU_GET_INFO_CALL(m37710);
}

DEFINE_LEGACY_CPU_DEVICE(M37710, m37710);
DEFINE_LEGACY_CPU_DEVICE(M37702, m37702);

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
