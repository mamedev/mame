// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell PPS-4/1 MCU cores

This is the single-chip evolution of Rockwell's older PPS-4 CPU. It is similar,
but a lot of things were simplified, the ALU instructions are less diverse.
It also has a lot in common with Rockwell's previous MCU series (A/B5000).

Part numbers:
- A75xx = MM75    - 28 pin dip
- A76xx = MM76    - 42 pin spider
- A77xx = MM77    - 42 pin spider
- A78xx = MM78    - 42 pin spider
- A79xx = MM76C   - 52 pin spider - high-speed counter
- A??xx = MM76D   - 52 pin spider - 12-bit ADC
- A86xx = MM76E   - 42 pin spider - extended ROM
- B76xx = MM76L   - 40 pin dip
- B77xx = MM77L   - 40 pin dip
- B80xx = MM77LA? - 40 pin dip
- B78xx = MM78L   - 40 pin dip
- B86xx = MM76EL  - 40 pin dip
- B90xx = MM78LA  - 42 pin spider

"spider" = 2 rows of pins on each side, just like standard PPS-4 CPUs.
"L" main difference is low-power

Internal clock is 4-phase (4 subcycles per 1-byte opcode), and when running
from an external oscillator, it is divided by 2 first. It also has an internal
oscillator which can be enabled with a resistor wired to VC.

References:
- Series MM76 Product Description
- Series MM77 Product Description
- MM76 Microcomputer Programming Manual
- MM77 Microcomputer Programming Manual

TODO:
- add extended opcodes to disasm? it's easy to add there, but the emulation goes
  through prefixes 1 cycle at the time which means the live disasm gets messy
- documentation discourages long jumps to the subroutine pages, but does not
  explain what would happen. Scrabble Sensor does it, so it's probably ok.
- documentation discourages use of some extended opcodes when in subroutine pages,
  but again does not explain why
- allowed opcode after TAB should be limited
- add MCU mask options, there's one for inverting interrupts
- does MM78LA support interrupts? the sparse documentation available says it does
- MM78LA mnemonics for changed opcodes is unknown
- no known documentation exists for MM77LA, mcu name is guessed (maybe it was
  designed in collaboration with Mattel, and later evolved into MM78LA)

*/

#include "emu.h"
#include "pps41base.h"


pps41_base_device::pps41_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth),
	m_opla(*this, "opla"),
	m_read_p(*this),
	m_read_d(*this),
	m_write_d(*this),
	m_read_r(*this),
	m_write_r(*this),
	m_read_sdi(*this),
	m_write_sdo(*this),
	m_write_ssc(*this),
	m_write_spk(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pps41_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks
	m_read_p.resolve_safe(0xff);
	m_read_d.resolve_safe(0);
	m_write_d.resolve_safe();
	m_read_r.resolve_safe(0xff);
	m_write_r.resolve_safe();
	m_read_sdi.resolve_safe(1);
	m_write_sdo.resolve_safe();
	m_write_ssc.resolve_safe();
	m_write_spk.resolve_safe();

	// init RAM with 0xf
	for (int i = 0; i <= m_datamask; i++)
		m_data->write_byte(i, 0xf);

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_prev2_op = 0;
	m_prev3_op = 0;
	memset(m_stack, 0, sizeof(m_stack));
	m_stack_levels = 1;

	m_a = 0;
	m_b = 0;
	m_prev_b = 0;
	m_prev2_b = 0;
	m_ram_addr = 0;
	m_ram_delay = false;
	m_sag = false;
	m_c = 0;
	m_prev_c = 0;
	m_c_in = 0;
	m_c_delay = false;
	m_x = 0;
	m_skip = false;
	m_skip_count = 0;

	m_s = 0;
	m_sclock_in = 0;
	m_sclock_count = 0;

	set_d_pins(10);
	m_d_output = 0;
	set_r_pins(8);
	m_r_output = 0;
	m_int_line[0] = m_int_line[1] = 1; // GND = 1
	m_int_ff[0] = m_int_ff[1] = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_prev2_op));
	save_item(NAME(m_prev3_op));
	save_item(NAME(m_stack));

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_prev_b));
	save_item(NAME(m_prev2_b));
	save_item(NAME(m_ram_addr));
	save_item(NAME(m_ram_delay));
	save_item(NAME(m_sag));
	save_item(NAME(m_c));
	save_item(NAME(m_prev_c));
	save_item(NAME(m_c_in));
	save_item(NAME(m_c_delay));
	save_item(NAME(m_x));
	save_item(NAME(m_skip));
	save_item(NAME(m_skip_count));

	save_item(NAME(m_s));
	save_item(NAME(m_sclock_in));
	save_item(NAME(m_sclock_count));

	save_item(NAME(m_d_output));
	save_item(NAME(m_r_output));
	save_item(NAME(m_int_line));
	save_item(NAME(m_int_ff));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prev_pc).formatstr("%03X").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%03X"); // 1
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 2
	state_add(++m_state_count, "C", m_c_in).formatstr("%01X"); // 3
	state_add(++m_state_count, "B", m_b).formatstr("%02X"); // 4
	state_add(++m_state_count, "S", m_s).formatstr("%01X"); // 5

	set_icountptr(m_icount);
}

device_memory_interface::space_config_vector pps41_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pps41_base_device::device_reset()
{
	m_op = m_prev_op = m_prev2_op = 0;
	m_pc = m_prgmask >> 1 & ~0x3f;
	m_skip = false;
	m_skip_count = 0;

	// clear outputs
	m_write_r(m_r_output = m_r_mask);
	m_write_d(m_d_output = 0);

	m_s = 0;
	m_sclock_count = 0;
	m_write_sdo(0);
	m_write_ssc(0);
}


//-------------------------------------------------
//  interrupt handling
//-------------------------------------------------

void pps41_base_device::execute_set_input(int line, int state)
{
	state = (state) ? 1 : 0;

	switch (line)
	{
		case PPS41_INPUT_LINE_INT0:
			// reset flip-flop on rising edge
			if (state && !m_int_line[0])
				m_int_ff[0] = 0;
			m_int_line[0] = state;
			break;

		case PPS41_INPUT_LINE_INT1:
			// reset flip-flop on falling edge
			if (!state && m_int_line[1])
				m_int_ff[1] = 0;
			m_int_line[1] = state;
			break;

		default:
			break;
	}
}


//-------------------------------------------------
//  serial i/o
//-------------------------------------------------

void pps41_base_device::ssc_w(int state)
{
	state = (state) ? 1 : 0;

	// serial shift on falling edge
	if (!state && m_sclock_in)
		serial_shift(m_read_sdi());
	m_sclock_in = state;
}

void pps41_base_device::serial_shift(int state)
{
	state = (state) ? 1 : 0;
	m_s = (m_s << 1 | state) & 0xf;
	m_write_sdo(BIT(m_s, 3));
}

void pps41_base_device::serial_clock()
{
	// internal serial clock cycle
	int i = m_read_sdi();
	m_sclock_count--;
	m_write_ssc(m_sclock_count & 1);

	if (~m_sclock_count & 1 && m_sclock_count < 8)
		serial_shift(i);
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void pps41_base_device::cycle()
{
	m_icount--;

	// clock serial i/o
	if (m_sclock_count)
		serial_clock();
}

void pps41_base_device::increment_pc()
{
	// low part is LFSR
	int feed = ((m_pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (m_pc >> 1 ^ m_pc) & 1;
	m_pc = (m_pc & ~0x3f) | (m_pc >> 1 & 0x1f) | (feed << 5);
}

void pps41_base_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev3_op = m_prev2_op;
		m_prev2_op = m_prev_op;
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		m_prev2_b = m_prev_b;
		m_prev_b = m_b;
		m_prev_c = m_c;

		// fetch next opcode
		if (!m_skip && !m_skip_count)
			debugger_instruction_hook(m_pc);
		m_op = m_program->read_byte(m_pc);
		increment_pc();
		cycle();

		// handle opcode if it's not skipped
		if (m_skip)
		{
			// still skip through prefix(es)
			m_skip = op_is_tr(m_op);
			m_op = 0; // fake nop
		}
		else if (m_skip_count)
		{
			m_skip_count--;

			// restore opcode state
			m_op = m_prev_op;
			m_prev_op = m_prev2_op;
			m_prev2_op = m_prev3_op;
		}
		else
			execute_one();

		// some opcodes delay RAM address(Bl part) adjustment for 1 cycle
		m_ram_addr = m_b;

		if (m_ram_delay)
		{
			m_ram_addr = (m_ram_addr & ~0xf) | (m_prev_b & 0xf);
			m_ram_delay = false;
		}

		// SAG sets RAM address(Bu part) to 3 for the next cycle
		if (m_sag)
		{
			m_ram_addr = (m_ram_addr & 0xf) | 0x30;
			m_sag = false;
		}

		// and some opcodes delay carry adjustment for 1 cycle
		m_c_in = m_c_delay ? m_prev_c : m_c;
		m_c_delay = false;
	}
}
