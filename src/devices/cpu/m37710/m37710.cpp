// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud, hap
/*
    Mitsubishi M37702/37710/37720 CPU Emulator

    The 7700 series is based on the WDC 65C816 core, with the following
    notable changes:

    - Second 16-bit accumulator called "B" (on the 65816, "A" and "B" were the
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
    - Unlike the 65C816, the program bank register (known here as PG) is
      incremented when PC overflows from 0xFFFF, and may be incremented or
      decremented when the address for a relative branch is calculated.
    - The external bus, if used, allows for 16-bit transfers, and can be
      dynamically reduced to 8 bits by asserting the BYTE input. (The
      65C816 has an 8-bit data bus.) Internal memory is also 16 bits wide,
      but parallel port registers must be accessed as individual bytes.

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

// verbose logging for peripherals, etc.
#define LOG_GENERAL (1U << 0)
#define LOG_PORTS (1U << 1)
#define LOG_AD (1U << 2)
#define LOG_UART (1U << 3)
#define LOG_TIMER (1U << 4)
#define LOG_INT (1U << 5)
//#define VERBOSE (LOG_GENERAL | LOG_PORTS | LOG_AD | LOG_UART | LOG_TIMER | LOG_INT)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(M37702M2, m37702m2_device, "m37702m2", "Mitsubishi M37702M2")
DEFINE_DEVICE_TYPE(M37702S1, m37702s1_device, "m37702s1", "Mitsubishi M37702S1")
DEFINE_DEVICE_TYPE(M37710S4, m37710s4_device, "m37710s4", "Mitsubishi M37710S4")
DEFINE_DEVICE_TYPE(M37720S1, m37720s1_device, "m37720s1", "Mitsubishi M37720S1")


// On-board RAM, ROM, and peripherals

// M37702M2: 512 bytes internal RAM, 16K internal mask ROM
// (M37702E2: same with EPROM instead of mask ROM)
void m37702m2_device::map(address_map &map)
{
	map(0x000002, 0x000015).rw(FUNC(m37702m2_device::port_r), FUNC(m37702m2_device::port_w));
	map(0x00001e, 0x00001e).rw(FUNC(m37702m2_device::ad_control_r), FUNC(m37702m2_device::ad_control_w));
	map(0x00001f, 0x00001f).rw(FUNC(m37702m2_device::ad_sweep_r), FUNC(m37702m2_device::ad_sweep_w));
	map(0x000020, 0x00002f).r(FUNC(m37702m2_device::ad_result_r));
	map(0x000030, 0x000030).rw(FUNC(m37702m2_device::uart0_mode_r), FUNC(m37702m2_device::uart0_mode_w));
	map(0x000031, 0x000031).w(FUNC(m37702m2_device::uart0_baud_w));
	map(0x000032, 0x000033).w(FUNC(m37702m2_device::uart0_tbuf_w));
	map(0x000034, 0x000034).rw(FUNC(m37702m2_device::uart0_ctrl_reg0_r), FUNC(m37702m2_device::uart0_ctrl_reg0_w));
	map(0x000035, 0x000035).rw(FUNC(m37702m2_device::uart0_ctrl_reg1_r), FUNC(m37702m2_device::uart0_ctrl_reg1_w));
	map(0x000036, 0x000037).r(FUNC(m37702m2_device::uart0_rbuf_r));
	map(0x000038, 0x000038).rw(FUNC(m37702m2_device::uart1_mode_r), FUNC(m37702m2_device::uart1_mode_w));
	map(0x000039, 0x000039).w(FUNC(m37702m2_device::uart1_baud_w));
	map(0x00003a, 0x00003b).w(FUNC(m37702m2_device::uart1_tbuf_w));
	map(0x00003c, 0x00003c).rw(FUNC(m37702m2_device::uart1_ctrl_reg0_r), FUNC(m37702m2_device::uart1_ctrl_reg0_w));
	map(0x00003d, 0x00003d).rw(FUNC(m37702m2_device::uart1_ctrl_reg1_r), FUNC(m37702m2_device::uart1_ctrl_reg1_w));
	map(0x00003e, 0x00003f).r(FUNC(m37702m2_device::uart1_rbuf_r));
	map(0x000040, 0x000040).rw(FUNC(m37702m2_device::count_start_r), FUNC(m37702m2_device::count_start_w));
	map(0x000042, 0x000042).w(FUNC(m37702m2_device::one_shot_start_w));
	map(0x000044, 0x000044).rw(FUNC(m37702m2_device::up_down_r), FUNC(m37702m2_device::up_down_w));
	map(0x000046, 0x000055).rw(FUNC(m37702m2_device::timer_reg_r), FUNC(m37702m2_device::timer_reg_w));
	map(0x000056, 0x00005d).rw(FUNC(m37702m2_device::timer_mode_r), FUNC(m37702m2_device::timer_mode_w));
	map(0x00005e, 0x00005e).rw(FUNC(m37702m2_device::proc_mode_r), FUNC(m37702m2_device::proc_mode_w));
	map(0x000060, 0x000060).w(FUNC(m37702m2_device::watchdog_timer_w));
	map(0x000061, 0x000061).rw(FUNC(m37702m2_device::watchdog_freq_r), FUNC(m37702m2_device::watchdog_freq_w));
	map(0x00006c, 0x00007f).rw(FUNC(m37702m2_device::int_control_r), FUNC(m37702m2_device::int_control_w));
	map(0x000080, 0x00027f).ram();
	map(0x00c000, 0x00ffff).rom().region(M37710_INTERNAL_ROM_REGION, 0);
}


// M37702S1: 512 bytes internal RAM, no internal ROM
void m37702s1_device::map(address_map &map)
{
	map(0x000002, 0x000015).rw(FUNC(m37702s1_device::port_r), FUNC(m37702s1_device::port_w));
	map(0x00001e, 0x00001e).rw(FUNC(m37702s1_device::ad_control_r), FUNC(m37702s1_device::ad_control_w));
	map(0x00001f, 0x00001f).rw(FUNC(m37702s1_device::ad_sweep_r), FUNC(m37702s1_device::ad_sweep_w));
	map(0x000020, 0x00002f).r(FUNC(m37702s1_device::ad_result_r));
	map(0x000030, 0x000030).rw(FUNC(m37702s1_device::uart0_mode_r), FUNC(m37702s1_device::uart0_mode_w));
	map(0x000031, 0x000031).w(FUNC(m37702s1_device::uart0_baud_w));
	map(0x000032, 0x000033).w(FUNC(m37702s1_device::uart0_tbuf_w));
	map(0x000034, 0x000034).rw(FUNC(m37702s1_device::uart0_ctrl_reg0_r), FUNC(m37702s1_device::uart0_ctrl_reg0_w));
	map(0x000035, 0x000035).rw(FUNC(m37702s1_device::uart0_ctrl_reg1_r), FUNC(m37702s1_device::uart0_ctrl_reg1_w));
	map(0x000036, 0x000037).r(FUNC(m37702s1_device::uart0_rbuf_r));
	map(0x000038, 0x000038).rw(FUNC(m37702s1_device::uart1_mode_r), FUNC(m37702s1_device::uart1_mode_w));
	map(0x000039, 0x000039).w(FUNC(m37702s1_device::uart1_baud_w));
	map(0x00003a, 0x00003b).w(FUNC(m37702s1_device::uart1_tbuf_w));
	map(0x00003c, 0x00003c).rw(FUNC(m37702s1_device::uart1_ctrl_reg0_r), FUNC(m37702s1_device::uart1_ctrl_reg0_w));
	map(0x00003d, 0x00003d).rw(FUNC(m37702s1_device::uart1_ctrl_reg1_r), FUNC(m37702s1_device::uart1_ctrl_reg1_w));
	map(0x00003e, 0x00003f).r(FUNC(m37702s1_device::uart1_rbuf_r));
	map(0x000040, 0x000040).rw(FUNC(m37702s1_device::count_start_r), FUNC(m37702s1_device::count_start_w));
	map(0x000042, 0x000042).w(FUNC(m37702s1_device::one_shot_start_w));
	map(0x000044, 0x000044).rw(FUNC(m37702s1_device::up_down_r), FUNC(m37702s1_device::up_down_w));
	map(0x000046, 0x000055).rw(FUNC(m37702s1_device::timer_reg_r), FUNC(m37702s1_device::timer_reg_w));
	map(0x000056, 0x00005d).rw(FUNC(m37702s1_device::timer_mode_r), FUNC(m37702s1_device::timer_mode_w));
	map(0x00005e, 0x00005e).rw(FUNC(m37702s1_device::proc_mode_r), FUNC(m37702s1_device::proc_mode_w));
	map(0x000060, 0x000060).w(FUNC(m37702s1_device::watchdog_timer_w));
	map(0x000061, 0x000061).rw(FUNC(m37702s1_device::watchdog_freq_r), FUNC(m37702s1_device::watchdog_freq_w));
	map(0x00006c, 0x00007f).rw(FUNC(m37702s1_device::int_control_r), FUNC(m37702s1_device::int_control_w));
	map(0x000080, 0x00027f).ram();
}


// M37710S4: 2048 bytes internal RAM, no internal ROM
void m37710s4_device::map(address_map &map)
{
	map(0x000002, 0x000015).rw(FUNC(m37710s4_device::port_r), FUNC(m37710s4_device::port_w)); // FIXME: P4-P8 only
	map(0x00001a, 0x00001d).w(FUNC(m37710s4_device::da_reg_w)).umask16(0x00ff);
	map(0x00001e, 0x00001e).rw(FUNC(m37710s4_device::ad_control_r), FUNC(m37710s4_device::ad_control_w));
	map(0x00001f, 0x00001f).rw(FUNC(m37710s4_device::ad_sweep_r), FUNC(m37710s4_device::ad_sweep_w));
	map(0x000020, 0x00002f).r(FUNC(m37710s4_device::ad_result_r));
	map(0x000030, 0x000030).rw(FUNC(m37710s4_device::uart0_mode_r), FUNC(m37710s4_device::uart0_mode_w));
	map(0x000031, 0x000031).w(FUNC(m37710s4_device::uart0_baud_w));
	map(0x000032, 0x000033).w(FUNC(m37710s4_device::uart0_tbuf_w));
	map(0x000034, 0x000034).rw(FUNC(m37710s4_device::uart0_ctrl_reg0_r), FUNC(m37710s4_device::uart0_ctrl_reg0_w));
	map(0x000035, 0x000035).rw(FUNC(m37710s4_device::uart0_ctrl_reg1_r), FUNC(m37710s4_device::uart0_ctrl_reg1_w));
	map(0x000036, 0x000037).r(FUNC(m37710s4_device::uart0_rbuf_r));
	map(0x000038, 0x000038).rw(FUNC(m37710s4_device::uart1_mode_r), FUNC(m37710s4_device::uart1_mode_w));
	map(0x000039, 0x000039).w(FUNC(m37710s4_device::uart1_baud_w));
	map(0x00003a, 0x00003b).w(FUNC(m37710s4_device::uart1_tbuf_w));
	map(0x00003c, 0x00003c).rw(FUNC(m37710s4_device::uart1_ctrl_reg0_r), FUNC(m37710s4_device::uart1_ctrl_reg0_w));
	map(0x00003d, 0x00003d).rw(FUNC(m37710s4_device::uart1_ctrl_reg1_r), FUNC(m37710s4_device::uart1_ctrl_reg1_w));
	map(0x00003e, 0x00003f).r(FUNC(m37710s4_device::uart1_rbuf_r));
	map(0x000040, 0x000040).rw(FUNC(m37710s4_device::count_start_r), FUNC(m37710s4_device::count_start_w));
	map(0x000042, 0x000042).w(FUNC(m37710s4_device::one_shot_start_w));
	map(0x000044, 0x000044).rw(FUNC(m37710s4_device::up_down_r), FUNC(m37710s4_device::up_down_w));
	map(0x000046, 0x000055).rw(FUNC(m37710s4_device::timer_reg_r), FUNC(m37710s4_device::timer_reg_w));
	map(0x000056, 0x00005d).rw(FUNC(m37710s4_device::timer_mode_r), FUNC(m37710s4_device::timer_mode_w));
	map(0x00005e, 0x00005e).rw(FUNC(m37710s4_device::proc_mode_r), FUNC(m37710s4_device::proc_mode_w));
	map(0x000060, 0x000060).w(FUNC(m37710s4_device::watchdog_timer_w));
	map(0x000061, 0x000061).rw(FUNC(m37710s4_device::watchdog_freq_r), FUNC(m37710s4_device::watchdog_freq_w));
	map(0x000062, 0x000062).rw(FUNC(m37710s4_device::waveform_mode_r), FUNC(m37710s4_device::waveform_mode_w));
	map(0x000064, 0x000065).w(FUNC(m37710s4_device::pulse_output_w));
	map(0x00006c, 0x00007f).rw(FUNC(m37710s4_device::int_control_r), FUNC(m37710s4_device::int_control_w));
	map(0x000080, 0x00087f).ram();
}

// M37720S1: 512 bytes internal RAM, no internal ROM, built-in DMA
void m37720s1_device::map(address_map &map)
{
	map(0x000002, 0x000019).rw(FUNC(m37720s1_device::port_r), FUNC(m37720s1_device::port_w)); // FIXME: P4-P10 only
	map(0x00001a, 0x00001d).w(FUNC(m37720s1_device::pulse_output_w)).umask16(0x00ff);
	map(0x00001e, 0x00001e).rw(FUNC(m37720s1_device::ad_control_r), FUNC(m37720s1_device::ad_control_w));
	map(0x00001f, 0x00001f).rw(FUNC(m37720s1_device::ad_sweep_r), FUNC(m37720s1_device::ad_sweep_w));
	map(0x000020, 0x00002f).r(FUNC(m37720s1_device::ad_result_r));
	map(0x000030, 0x000030).rw(FUNC(m37720s1_device::uart0_mode_r), FUNC(m37720s1_device::uart0_mode_w));
	map(0x000031, 0x000031).w(FUNC(m37720s1_device::uart0_baud_w));
	map(0x000032, 0x000033).w(FUNC(m37720s1_device::uart0_tbuf_w));
	map(0x000034, 0x000034).rw(FUNC(m37720s1_device::uart0_ctrl_reg0_r), FUNC(m37720s1_device::uart0_ctrl_reg0_w));
	map(0x000035, 0x000035).rw(FUNC(m37720s1_device::uart0_ctrl_reg1_r), FUNC(m37720s1_device::uart0_ctrl_reg1_w));
	map(0x000036, 0x000037).r(FUNC(m37720s1_device::uart0_rbuf_r));
	map(0x000038, 0x000038).rw(FUNC(m37720s1_device::uart1_mode_r), FUNC(m37720s1_device::uart1_mode_w));
	map(0x000039, 0x000039).w(FUNC(m37720s1_device::uart1_baud_w));
	map(0x00003a, 0x00003b).w(FUNC(m37720s1_device::uart1_tbuf_w));
	map(0x00003c, 0x00003c).rw(FUNC(m37720s1_device::uart1_ctrl_reg0_r), FUNC(m37720s1_device::uart1_ctrl_reg0_w));
	map(0x00003d, 0x00003d).rw(FUNC(m37720s1_device::uart1_ctrl_reg1_r), FUNC(m37720s1_device::uart1_ctrl_reg1_w));
	map(0x00003e, 0x00003f).r(FUNC(m37720s1_device::uart1_rbuf_r));
	map(0x000040, 0x000040).rw(FUNC(m37720s1_device::count_start_r), FUNC(m37720s1_device::count_start_w));
	map(0x000042, 0x000042).w(FUNC(m37720s1_device::one_shot_start_w));
	map(0x000044, 0x000044).rw(FUNC(m37720s1_device::up_down_r), FUNC(m37720s1_device::up_down_w));
	map(0x000046, 0x000055).rw(FUNC(m37720s1_device::timer_reg_r), FUNC(m37720s1_device::timer_reg_w));
	map(0x000056, 0x00005d).rw(FUNC(m37720s1_device::timer_mode_r), FUNC(m37720s1_device::timer_mode_w));
	map(0x00005e, 0x00005e).rw(FUNC(m37720s1_device::proc_mode_r), FUNC(m37720s1_device::proc_mode_w));
	map(0x000060, 0x000060).w(FUNC(m37720s1_device::watchdog_timer_w));
	map(0x000061, 0x000061).rw(FUNC(m37720s1_device::watchdog_freq_r), FUNC(m37720s1_device::watchdog_freq_w));
	map(0x000062, 0x000062).rw(FUNC(m37720s1_device::rto_control_r), FUNC(m37720s1_device::rto_control_w));
	map(0x000064, 0x000064).rw(FUNC(m37720s1_device::dram_control_r), FUNC(m37720s1_device::dram_control_w));
	map(0x000066, 0x000066).w(FUNC(m37720s1_device::refresh_timer_w));
	map(0x000068, 0x000069).rw(FUNC(m37720s1_device::dmac_control_r), FUNC(m37720s1_device::dmac_control_w));
	map(0x00006c, 0x00007f).rw(FUNC(m37720s1_device::int_control_r), FUNC(m37720s1_device::int_control_w));
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

const int m37710_cpu_device::m37710_int_reg_map[M37710_MASKABLE_INTERRUPTS] =
{
	M37710_LINE_DMA0,       // level 3  (0x6c)
	M37710_LINE_DMA1,       // level 2  (0x6d)
	M37710_LINE_DMA2,       // level 1  (0x6e)
	M37710_LINE_DMA3,       // level 0  (0x6f)
	M37710_LINE_ADC,        // level 4  (0x70)
	M37710_LINE_UART0XMIT,  // level 7  (0x71)
	M37710_LINE_UART0RECV,  // level 8  (0x72)
	M37710_LINE_UART1XMIT,  // level 5  (0x73)
	M37710_LINE_UART1RECV,  // level 6  (0x74)
	M37710_LINE_TIMERA0,    // level 16 (0x75)
	M37710_LINE_TIMERA1,    // level 15 (0x76)
	M37710_LINE_TIMERA2,    // level 14 (0x77)
	M37710_LINE_TIMERA3,    // level 13 (0x78)
	M37710_LINE_TIMERA4,    // level 12 (0x79)
	M37710_LINE_TIMERB0,    // level 11 (0x7a)
	M37710_LINE_TIMERB1,    // level 10 (0x7b)
	M37710_LINE_TIMERB2,    // level 9  (0x7c)
	M37710_LINE_IRQ0,       // level 19 (0x7d)
	M37710_LINE_IRQ1,       // level 18 (0x7e)
	M37710_LINE_IRQ2,       // level 17 (0x7f)
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

const char *const m37710_cpu_device::m37710_tnames[8] =
{
	"A0", "A1", "A2", "A3", "A4", "B0", "B1", "B2"
};

const char *const m37710_cpu_device::m37710_intnames[M37710_INTERRUPT_MAX] =
{
	"DMA3", "DMA2", "DMA1", "DMA0",
	"A/D",
	"UART1 xmit", "UART1 recv",
	"UART0 xmit", "UART0 recv",
	"Timer B2", "Timer B1", "Timer B0",
	"Timer A4", "Timer A3", "Timer A2", "Timer A1", "Timer A0",
	"INT2", "INT1", "INT0",
	"Watchdog timer", "DBC", "BRK", "Zero division", "RESET" // nonmaskable
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
	if (BIT(m_count_start, timer))
	{
		if ((m_timer_mode[timer] & 0x3) == 1)
		{
			int upcount = 0;

			// timer b always counts down
			if (timer <= 4)
			{
				if (BIT(m_timer_mode[timer], 4))
				{
					// up/down determined by timer out pin
					upcount = m_timer_out[timer];
				}
				else
					upcount = m_up_down_reg >> timer & 1;
			}

			if (upcount)
				m_timer_reg[timer]++;
			else
				m_timer_reg[timer]--;
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
	if (BIT(m_count_start, timer))
	{
		LOGMASKED(LOG_TIMER, "Timer %d (%s) is enabled\n", timer, m37710_tnames[timer]);

		// set the timer's value
		tval = m_timer_reg[timer];

		// HACK: ignore if timer is 8MHz (MAME slows down to a crawl)
		if (tval == 0 && (m_timer_mode[timer]&0xc0) == 0) return;

		// check timer's mode
		// modes are slightly different between timer groups A and B
		if (timer < 5)
		{
			switch (m_timer_mode[timer] & 0x3)
			{
				case 0:         // timer mode
					time = clocks_to_attotime(tscales[m_timer_mode[timer]>>6]);
					time *= (tval + 1);

					LOGMASKED(LOG_TIMER, "Timer %d in timer mode, %f Hz\n", timer, 1.0 / time.as_double());

					m_timers[timer]->adjust(time, timer);
					m_reload[timer] = time;
					break;

				case 1:         // event counter mode
					LOGMASKED(LOG_TIMER, "Timer %d in event counter mode\n", timer);
					break;

				case 2:     // one-shot pulse mode
					LOGMASKED(LOG_TIMER, "Timer %d in one-shot mode\n", timer);
					break;

				case 3:         // PWM mode
					LOGMASKED(LOG_TIMER, "Timer %d in PWM mode\n", timer);
					break;
			}
		}
		else
		{
			switch (m_timer_mode[timer] & 0x3)
			{
				case 0:         // timer mode
					time = clocks_to_attotime(tscales[m_timer_mode[timer]>>6]);
					time *= (tval + 1);

					LOGMASKED(LOG_TIMER, "Timer %d in timer mode, %f Hz\n", timer, 1.0 / time.as_double());

					m_timers[timer]->adjust(time, timer);
					m_reload[timer] = time;
					break;

				case 1:         // event counter mode
					LOGMASKED(LOG_TIMER, "Timer %d in event counter mode\n", timer);
					break;

				case 2:     // pulse period/pulse width measurement mode
					LOGMASKED(LOG_TIMER, "Timer %d in pulse period/width measurement mode\n", timer);
					break;

				case 3:
					LOGMASKED(LOG_TIMER, "Timer %d in unknown mode!\n", timer);
					break;
			}
		}
	}
}

uint8_t m37710_cpu_device::port_r(offs_t offset)
{
	int p = (offset & 0x1c) >> 1 | (offset & 0x01);

	uint8_t result = 0;
	if (p < 11)
	{
		if (BIT(offset, 1))
			result = m_port_dir[p];
		else
		{
			uint8_t d = m_port_dir[p];
			if (d != 0xff)
				result = (m_port_in_cb[p](0, ~d) & ~d) | (m_port_regs[p] & d);
			else
				result = m_port_regs[p];
		}
	}

	LOGMASKED(LOG_PORTS, "port_r from %02x: Port P%d %s = %x\n", (int)offset + 0x02, p, BIT(offset, 1) ? "dir reg" : "reg", result);

	return result;
}

void m37710_cpu_device::port_w(offs_t offset, uint8_t data)
{
	int p = (offset & 0x1c) >> 1 | (offset & 0x01);

	if (p < 11)
	{
		LOGMASKED(LOG_PORTS, "port_w %x to %02x: Port P%d %s = %x\n", data, (int)offset + 0x02, p,
			BIT(offset, 1) ? "dir reg" : "reg",
			BIT(offset, 1) ? m_port_dir[p] : m_port_regs[p]);

		if (BIT(offset, 1))
			m_port_dir[p] = data;
		else
		{
			uint8_t d = m_port_dir[p];
			if (d != 0)
				m_port_out_cb[p](0, data & d, d);
			m_port_regs[p] = data;
		}
	}
}

void m37710_cpu_device::da_reg_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "da_reg_w %x to %02X: D/A %d\n", data, (int)(offset * 2) + 0x1a, offset);
}

void m37710_cpu_device::pulse_output_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "pulse_output_w %x: Pulse output data register %d\n", data, offset);
}

uint8_t m37710_cpu_device::ad_control_r()
{
	return m_ad_control;
}

void m37710_cpu_device::ad_control_w(uint8_t data)
{
	LOGMASKED(LOG_AD, "ad_control_w %x: A/D control reg = %x\n", data, m_ad_control);

	if (BIT(data, 6) && !BIT(m_ad_control, 6))
	{
		// A-D conversion clock may be selected as f2/4 or f2/2
		m_ad_timer->adjust(clocks_to_attotime(57 * 2 * (BIT(data, 7) ? 2 : 4)));
		if (BIT(data, 4))
			data &= 0xf8;
	}
	else if (!BIT(data, 6))
		m_ad_timer->adjust(attotime::never);

	m_ad_control = data;
}

TIMER_CALLBACK_MEMBER(m37710_cpu_device::ad_timer_cb)
{
	int line = m_ad_control & 0x07;

	m_ad_result[line] = m_analog_cb[line]();

	if (BIT(m_ad_control, 4))
		m_ad_control = (m_ad_control & 0xf8) | ((line + 1) & 0x07);

	// repeat or sweep conversion
	if (BIT(m_ad_control, 3) || (BIT(m_ad_control, 4) && line != (m_ad_sweep & 0x03) * 2 + 1))
	{
		LOGMASKED(LOG_AD, "AN%d input converted = %x (repeat/sweep)\n", line, m_ad_result[line]);
		m_ad_timer->adjust(clocks_to_attotime(57 * 2 * (BIT(m_ad_control, 7) ? 2 : 4)));
	}
	else
	{
		// interrupt occurs only when conversion stops
		LOGMASKED(LOG_AD, "AN%d input converted = %x (finished)\n", line, m_ad_result[line]);
		m37710_set_irq_line(M37710_LINE_ADC, ASSERT_LINE);
		m_ad_control &= 0xbf;
	}
}

uint8_t m37710_cpu_device::ad_sweep_r()
{
	return m_ad_sweep;
}

void m37710_cpu_device::ad_sweep_w(uint8_t data)
{
	LOGMASKED(LOG_AD, "ad_sweep_w %x: A/D sweep pin select = %x\n", data, m_ad_sweep);

	m_ad_sweep = data;
}

uint16_t m37710_cpu_device::ad_result_r(offs_t offset)
{
	uint16_t result = m_ad_result[offset];

	LOGMASKED(LOG_AD, "ad_result_r from %02x: A/D %d = %x (PC=%x)\n", (int)(offset * 2) + 0x20, offset, result, REG_PB<<16 | REG_PC);

	return result;
}

uint8_t m37710_cpu_device::uart0_mode_r()
{
	LOGMASKED(LOG_UART, "uart0_mode_r: UART0 transmit/recv mode = %x (PC=%x)\n", m_uart_mode[0], REG_PB<<16 | REG_PC);

	return m_uart_mode[0];
}

void m37710_cpu_device::uart0_mode_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart0_mode_w %x: UART0 transmit/recv mode = %x\n", data, m_uart_mode[0]);

	m_uart_mode[0] = data;
}

uint8_t m37710_cpu_device::uart1_mode_r()
{
	LOGMASKED(LOG_UART, "uart1_mode_r: UART1 transmit/recv mode = %x (PC=%x)\n", m_uart_mode[1], REG_PB<<16 | REG_PC);

	return m_uart_mode[1];
}

void m37710_cpu_device::uart1_mode_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart1_mode_w %x: UART1 transmit/recv mode = %x\n", data, m_uart_mode[1]);

	m_uart_mode[1] = data;
}

void m37710_cpu_device::uart0_baud_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart0_baud_w %x: UART0 baud rate = %x\n", data, m_uart_baud[0]);

	m_uart_baud[0] = data;
}

void m37710_cpu_device::uart1_baud_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart1_baud_w %x: UART1 baud rate = %x\n", data, m_uart_baud[1]);

	m_uart_baud[1] = data;
}

void m37710_cpu_device::uart0_tbuf_w(uint16_t data)
{
	LOGMASKED(LOG_UART, "uart0_tbuf_w %x: UART0 transmit buf\n", data);
}

void m37710_cpu_device::uart1_tbuf_w(uint16_t data)
{
	LOGMASKED(LOG_UART, "uart1_tbuf_w %x: UART1 transmit buf\n", data);
}

uint8_t m37710_cpu_device::uart0_ctrl_reg0_r()
{
	LOGMASKED(LOG_UART, "uart0_ctrl_reg0_r: UART0 transmit/recv ctrl 0 = %x (PC=%x)\n", m_uart_ctrl_reg0[0], REG_PB<<16 | REG_PC);

	return m_uart_ctrl_reg0[0];
}

void m37710_cpu_device::uart0_ctrl_reg0_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart0_ctrl_reg0_w %x: UART0 transmit/recv ctrl 0 = %x\n", data, m_uart_ctrl_reg0[0]);

	// Tx empty flag is read-only
	m_uart_ctrl_reg0[0] = (data & ~8) | (m_uart_ctrl_reg0[0] & 8);
}

uint8_t m37710_cpu_device::uart1_ctrl_reg0_r()
{
	LOGMASKED(LOG_UART, "uart1_ctrl_reg0_r: UART1 transmit/recv ctrl 0 = %x (PC=%x)\n", m_uart_ctrl_reg0[1], REG_PB<<16 | REG_PC);

	return m_uart_ctrl_reg0[1];
}

void m37710_cpu_device::uart1_ctrl_reg0_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart1_ctrl_reg0_w %x: UART1 transmit/recv ctrl 0 = %x\n", data, m_uart_ctrl_reg0[1]);

	// Tx empty flag is read-only
	m_uart_ctrl_reg0[1] = (data & ~8) | (m_uart_ctrl_reg0[1] & 8);
}

uint8_t m37710_cpu_device::uart0_ctrl_reg1_r()
{
	LOGMASKED(LOG_UART, "uart0_ctrl_reg1_r: UART0 transmit/recv ctrl 1 = %x (PC=%x)\n", m_uart_ctrl_reg1[0], REG_PB<<16 | REG_PC);

	return m_uart_ctrl_reg1[0];
}

void m37710_cpu_device::uart0_ctrl_reg1_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart0_ctrl_reg1_w %x: UART0 transmit/recv ctrl 1 = %x\n", data, m_uart_ctrl_reg1[0]);

	m_uart_ctrl_reg1[0] = (m_uart_ctrl_reg1[0] & (BIT(data, 2) ? 0xfa : 0x0a)) | (data & 0x05);
}

uint8_t m37710_cpu_device::uart1_ctrl_reg1_r()
{
	LOGMASKED(LOG_UART, "uart1_ctrl_reg1_r: UART1 transmit/recv ctrl 1 = %x (PC=%x)\n", m_uart_ctrl_reg1[1], REG_PB<<16 | REG_PC);

	return m_uart_ctrl_reg1[1];
}

void m37710_cpu_device::uart1_ctrl_reg1_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "uart1_ctrl_reg1_w %x: UART1 transmit/recv ctrl 1 = %x\n", data, m_uart_ctrl_reg1[1]);

	m_uart_ctrl_reg1[1] = (m_uart_ctrl_reg1[1] & (BIT(data, 2) ? 0xfa : 0x0a)) | (data & 0x05);
}

uint16_t m37710_cpu_device::uart0_rbuf_r()
{
	LOGMASKED(LOG_UART, "uart0_rbuf_r: UART0 recv buf (PC=%x)\n", REG_PB<<16 | REG_PC);

	return 0;
}

uint16_t m37710_cpu_device::uart1_rbuf_r()
{
	LOGMASKED(LOG_UART, "uart1_rbuf_r: UART1 recv buf (PC=%x)\n", REG_PB<<16 | REG_PC);

	return 0;
}

uint8_t m37710_cpu_device::count_start_r()
{
	LOGMASKED(LOG_TIMER, "count_start_r: Count start = %x (PC=%x)\n", m_count_start, REG_PB<<16 | REG_PC);

	return m_count_start;
}

void m37710_cpu_device::count_start_w(uint8_t data)
{
	uint8_t prevdata = m_count_start;
	m_count_start = data;

	LOGMASKED(LOG_TIMER, "count_start_w %x: Count start = %x\n", data, prevdata);

	for (int i = 0; i < 8; i++)
		if (BIT(data, i) && !BIT(prevdata, i))
			m37710_recalc_timer(i);
}

void m37710_cpu_device::one_shot_start_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER, "one_shot_start_w %x: One-shot start = %x\n", data, m_one_shot_start);

	m_one_shot_start = data;
}

uint8_t m37710_cpu_device::up_down_r()
{
	LOGMASKED(LOG_TIMER, "up_down_r: Up-down register = %x (PC=%x)\n", m_up_down_reg, REG_PB<<16 | REG_PC);

	// bits 7-5 read back as 0
	return m_up_down_reg & 0x1f;
}

void m37710_cpu_device::up_down_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER, "up_down_w %x: Up-down register = %x\n", data, m_up_down_reg);

	m_up_down_reg = data;
}

uint16_t m37710_cpu_device::timer_reg_r(offs_t offset, uint16_t mem_mask)
{
	return m_timer_reg[offset] & mem_mask;
}

void m37710_cpu_device::timer_reg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_TIMER, "timer_reg_w %04x & %04x to %02x: Timer %s = %04x\n", data, mem_mask, (int)(offset * 2) + 0x46, m37710_tnames[offset], m_timer_reg[offset]);

	m_timer_reg[offset] = (data & mem_mask) | (m_timer_reg[offset] & ~mem_mask);
}

uint8_t m37710_cpu_device::timer_mode_r(offs_t offset)
{
	LOGMASKED(LOG_TIMER, "timer_mode_r from %02x: Timer %s mode = %x (PC=%x)\n", (int)offset + 0x56, m37710_tnames[offset], m_timer_mode[offset], REG_PB<<16 | REG_PC);

	return m_timer_mode[offset];
}

void m37710_cpu_device::timer_mode_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_TIMER, "timer_mode_w %x to %02x: Timer %s mode = %x\n", data, (int)offset + 0x56, m37710_tnames[offset], m_timer_mode[offset]);

	m_timer_mode[offset] = data;
}

uint8_t m37710_cpu_device::proc_mode_r(offs_t offset)
{
	LOGMASKED(LOG_GENERAL, "proc_mode_r: Processor mode = %x (PC=%x)\n", m_proc_mode, REG_PB<<16 | REG_PC);

	return m_proc_mode & 0xf7;
}

void m37710_cpu_device::proc_mode_w(uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "proc_mode_w %x: Processor mode = %x\n", data, m_proc_mode);

	m_proc_mode = data;
}

void m37710_cpu_device::watchdog_timer_w(uint8_t data)
{
	// TODO: reset watchdog timer (data is irrelevant)
}

uint8_t m37710_cpu_device::watchdog_freq_r()
{
	return m_watchdog_freq;
}

void m37710_cpu_device::watchdog_freq_w(uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "watchdog_freq_w %x: Watchdog timer frequency = %x\n", data, m_watchdog_freq);

	m_watchdog_freq = data;
}

uint8_t m37710_cpu_device::waveform_mode_r()
{
	LOGMASKED(LOG_GENERAL, "waveform_mode_r: Waveform output mode (PC=%x)\n", REG_PB<<16 | REG_PC);

	return 0;
}

void m37710_cpu_device::waveform_mode_w(uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "waveform_mode_w %x: Waveform output mode\n", data);
}

uint8_t m37710_cpu_device::rto_control_r()
{
	LOGMASKED(LOG_GENERAL, "rto_control_r: Real-time output control = %x (PC=%x)\n", m_rto_control, REG_PB<<16 | REG_PC);

	return m_rto_control;
}

void m37710_cpu_device::rto_control_w(uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "rto_control_w %x: Real-time output control = %x\n", data, m_rto_control);

	m_rto_control = data;
}

uint8_t m37710_cpu_device::dram_control_r()
{
	LOGMASKED(LOG_GENERAL, "dram_control_r: DRAM control = %x (PC=%x)\n", m_dram_control, REG_PB<<16 | REG_PC);

	return m_dram_control;
}

void m37710_cpu_device::dram_control_w(uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "dram_control_w %x: DRAM control = %x\n", data, m_dram_control);

	m_dram_control = data;
}

void m37710_cpu_device::refresh_timer_w(uint8_t data)
{
	LOGMASKED(LOG_GENERAL, "refresh_timer_w %x: Set refresh timer\n", data);
}

uint16_t m37710_cpu_device::dmac_control_r(offs_t offset, uint16_t mem_mask)
{
	return m_dmac_control & mem_mask;
}

void m37710_cpu_device::dmac_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_GENERAL, "dmac_control_w %04x & %04x: DMAC control = %04x\n", data, mem_mask, m_dmac_control);

	m_dmac_control = (data & mem_mask) | (m_timer_reg[offset] & ~mem_mask);
}

uint8_t m37710_cpu_device::int_control_r(offs_t offset)
{
	assert(offset < M37710_MASKABLE_INTERRUPTS);
	int level = m37710_int_reg_map[offset];

	//LOGMASKED(LOG_INT, "int_control_r from %02x: %s IRQ ctrl = %x (PC=%x)\n", (int)offset + 0x6c, m37710_intnames[level], m_int_control[level], REG_PB<<16 | REG_PC);

	uint8_t result = m_int_control[level];

	return result;
}

void m37710_cpu_device::int_control_w(offs_t offset, uint8_t data)
{
	assert(offset < M37710_MASKABLE_INTERRUPTS);
	int level = m37710_int_reg_map[offset];

	LOGMASKED(LOG_INT, "int_control_w %x to %02x: %s IRQ ctrl = %x\n", data, (int)offset + 0x6c, m37710_intnames[level], m_int_control[level]);

	m_int_control[level] = data;

	//m37710_set_irq_line(offset, (data & 8) ? HOLD_LINE : CLEAR_LINE);
	m37710i_update_irqs();

	// level-sense interrupts are not implemented yet
	if ((level == M37710_LINE_IRQ0 || level == M37710_LINE_IRQ1 || level == M37710_LINE_IRQ2) && BIT(data, 5))
		logerror("error M37710: INT%d level-sense\n", M37710_LINE_IRQ0 - level);
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
			if (curirq < M37710_MASKABLE_INTERRUPTS)
			{
				int control = m_int_control[curirq];
				int thispri = control & 7;
				// logerror("line %d set, level %x curpri %x IPL %x\n", curirq, thispri, curpri, m_ipl);
				// it's maskable, check if the level works, also make sure it's acceptable for the current CPU level
				if (!FLAG_I && thispri > curpri && thispri > m_ipl)
				{
					// mark us as the best candidate
					LOGMASKED(LOG_INT, "%s interrupt active with priority %d (PC=%x)\n", m37710_intnames[curirq], thispri, REG_PB<<16 | REG_PC);
					wantedIRQ = curirq;
					curpri = thispri;
				}
			}
			else
			{
				// non-maskable
				LOGMASKED(LOG_INT, "%s interrupt active (PC=%x)\n", m37710_intnames[curirq], REG_PB<<16 | REG_PC);
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
	std::fill(std::begin(m_port_dir), std::end(m_port_dir), 0);

	// A-D
	m_ad_control &= 7;
	m_ad_sweep = (m_ad_sweep & ~0xdc) | 3;
	m_ad_timer->reset();

	// UARTs
	for (i = 0; i < 2; i++)
	{
		m_uart_mode[i] = 0;
		m_uart_ctrl_reg0[i] = (m_uart_ctrl_reg0[i] & 0xe0) | 8;
		m_uart_ctrl_reg1[i] = 2;
	}

	// timers
	m_count_start = 0;
	m_one_shot_start &= ~0x1f;
	m_up_down_reg = 0;
	for (i = 0; i < 8; i++)
		m_timer_mode[i] = 0;

	m_proc_mode = 0; // processor mode
	m_watchdog_freq &= ~1; // watchdog timer frequency
	m_rto_control &= ~3;
	m_dram_control &= ~0x8f;

	// interrupt control
	for (i = 0; i <= M37710_LINE_TIMERA0; i++)
		m_int_control[i] &= ~0xf;
	for (i = M37710_LINE_IRQ2; i <= M37710_LINE_IRQ0; i++)
		m_int_control[i] &= ~0x3f;

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
	switch(line)
	{
		// maskable interrupts
		case M37710_LINE_ADC:
		case M37710_LINE_UART1XMIT:
		case M37710_LINE_UART1RECV:
		case M37710_LINE_UART0XMIT:
		case M37710_LINE_UART0RECV:
		case M37710_LINE_TIMERB2:
		case M37710_LINE_TIMERB1:
		case M37710_LINE_TIMERB0:
		case M37710_LINE_TIMERA4:
		case M37710_LINE_TIMERA3:
		case M37710_LINE_TIMERA2:
		case M37710_LINE_TIMERA1:
		case M37710_LINE_TIMERA0:
		case M37710_LINE_IRQ2:
		case M37710_LINE_IRQ1:
		case M37710_LINE_IRQ0:
		case M37710_LINE_DMA0:
		case M37710_LINE_DMA1:
		case M37710_LINE_DMA2:
		case M37710_LINE_DMA3:
			switch(state)
			{
				case CLEAR_LINE:
					LINE_IRQ &= ~(1 << line);
					m_int_control[line] &= ~8;
					break;

				case ASSERT_LINE:
				case HOLD_LINE:
					LINE_IRQ |= (1 << line);
					m_int_control[line] |= 8;
					break;

				default: break;
			}
			break;

		default: break;
	}
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
	m_stopped = 0;
	std::fill(std::begin(m_port_regs), std::end(m_port_regs), 0);
	std::fill(std::begin(m_port_dir), std::end(m_port_dir), 0);
	m_ad_control = 0;
	m_ad_sweep = 0;
	std::fill(std::begin(m_ad_result), std::end(m_ad_result), 0);
	std::fill(std::begin(m_uart_mode), std::end(m_uart_mode), 0);
	std::fill(std::begin(m_uart_baud), std::end(m_uart_baud), 0);
	std::fill(std::begin(m_uart_ctrl_reg0), std::end(m_uart_ctrl_reg0), 0);
	std::fill(std::begin(m_uart_ctrl_reg1), std::end(m_uart_ctrl_reg1), 0);
	m_count_start = 0;
	m_one_shot_start = 0;
	m_up_down_reg = 0;
	std::fill(std::begin(m_timer_reg), std::end(m_timer_reg), 0);
	std::fill(std::begin(m_timer_mode), std::end(m_timer_mode), 0);
	m_proc_mode = 0;
	m_watchdog_freq = 0;
	std::fill(std::begin(m_int_control), std::end(m_int_control), 0);

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

	m_ad_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m37710_cpu_device::ad_timer_cb), this));

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
	save_item(NAME(m_stopped));
	save_item(NAME(m_port_regs));
	save_item(NAME(m_port_dir));
	save_item(NAME(m_ad_control));
	save_item(NAME(m_ad_sweep));
	save_item(NAME(m_ad_result));
	save_item(NAME(m_uart_mode));
	save_item(NAME(m_uart_baud));
	save_item(NAME(m_uart_ctrl_reg0));
	save_item(NAME(m_uart_ctrl_reg1));
	save_item(NAME(m_count_start));
	save_item(NAME(m_one_shot_start));
	save_item(NAME(m_up_down_reg));
	save_item(NAME(m_timer_reg));
	save_item(NAME(m_timer_mode));
	save_item(NAME(m_reload[0]));
	save_item(NAME(m_reload[1]));
	save_item(NAME(m_reload[2]));
	save_item(NAME(m_reload[3]));
	save_item(NAME(m_reload[4]));
	save_item(NAME(m_reload[5]));
	save_item(NAME(m_reload[6]));
	save_item(NAME(m_reload[7]));
	save_item(NAME(m_timer_out));
	save_item(NAME(m_proc_mode));
	save_item(NAME(m_watchdog_freq));
	save_item(NAME(m_int_control));

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
