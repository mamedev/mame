// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC RX01 skeleton CPU device

    This TTL disk control processor executes its custom instruction set at
    the rather brisk rate of 200 ns per machine cycle. However, it has no
    ALU or general-purpose data bus, so most of its operations amount to
    simple manipulations of an assortment of synchronous up counters, shift
    registers and flip-flops, plus a 16-location scratchpad made up of two
    7489 16x4 register files.

    The instruction memory is organized as a series of 256-byte "fields"
    which limit the extent of conditional branches. The architecture allows
    for up to 16 fields, although the original hardware only implements F0
    through F5. DEC's documentation treats the program counter as being
    only 8 bits, calling the upper 4 bits the field counter. This emulation
    treats PC and FC as a single 12-bit register since the overflow carry
    from the lower counters is in fact linked to the upper counter, even
    though the actual microcode does not rely on this.

    The CRC LFSR is implemented using three 74174 registers using negative
    logic, since these TTL ICs can be cleared but not preset.

***************************************************************************/

#include "emu.h"
#include "rx01.h"
#include "rx01dasm.h"

//#define VERBOSE 1
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(RX01_CPU, rx01_cpu_device, "rx01_cpu", "DEC RX01 CPU")

rx01_cpu_device::rx01_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, RX01_CPU, tag, owner, clock)
	, m_inst_config("program", ENDIANNESS_LITTLE, 8, 12, 0)
	, m_data_config("sectordata", ENDIANNESS_LITTLE, 8, 10, 0) // actually 1 bit wide
	, m_interface_callback(*this)
	, m_pc(0)
	, m_ppc(0)
	, m_mb(0)
	, m_br_condition(false)
	, m_inst_disable(false)
	, m_inst_repeat(false)
	, m_cntr(0)
	, m_sr(0)
	, m_spar(0)
	, m_bar(0)
	, m_crc(0)
	, m_flags(0)
	, m_run(false)
	, m_12_bit(false)
	, m_data_in(false)
	, m_unit(false)
	, m_load_head(false)
	, m_syn_index(false)
	, m_icount(0)
{
	std::fill(std::begin(m_sp), std::end(m_sp), 0);
	m_inst_config.m_is_octal = true;
}

std::unique_ptr<util::disasm_interface> rx01_cpu_device::create_disassembler()
{
	return std::make_unique<rx01_disassembler>();
}

device_memory_interface::space_config_vector rx01_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_inst_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void rx01_cpu_device::device_resolve_objects()
{
	m_interface_callback.resolve_all_safe();
}

void rx01_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_inst_cache);
	space(AS_DATA).cache(m_data_cache);

	set_icountptr(m_icount);

	// Debug state registration
	state_add(RX01_PC, "PC", m_pc).mask(07777).formatstr("%04O");
	state_add(STATE_GENPC, "GENPC", m_pc).mask(07777).formatstr("%04O").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(07777).formatstr("%04O").noshow();
	state_add(STATE_GENFLAGS, "FLAGS", m_flags).formatstr("%12s").noshow();
	state_add(RX01_CNTR, "CNTR", m_cntr).formatstr("%03O");
	state_add(RX01_SR, "SR", m_sr).formatstr("%03O");
	state_add(RX01_SPAR, "SPAR", m_spar).mask(15).formatstr("%3s");
	for (int r = 0; r < 16; r++)
		state_add(RX01_R0 + r, string_format("R%d", r).c_str(), m_sp[r]).formatstr("%03O");
	state_add(RX01_BAR, "BAR", m_bar).mask(07777).formatstr("%04O");
	state_add(RX01_CRC, "CRC", m_crc).formatstr("%06O");
	state_add(RX01_UNIT, "UNIT", m_unit);
	state_add(RX01_LDHD, "LDHD", m_load_head);
	state_add(RX01_INDEX, "INDEX", m_syn_index);

	// Save state registration
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_mb));
	save_item(NAME(m_br_condition));
	save_item(NAME(m_inst_disable));
	save_item(NAME(m_inst_repeat));
	save_item(NAME(m_cntr));
	save_item(NAME(m_sr));
	save_item(NAME(m_spar));
	save_item(NAME(m_bar));
	save_item(NAME(m_crc));
	save_item(NAME(m_flags));
	save_item(NAME(m_run));
	save_item(NAME(m_12_bit));
	save_item(NAME(m_data_in));
	save_item(NAME(m_unit));
	save_item(NAME(m_load_head));
	save_item(NAME(m_syn_index));
}

void rx01_cpu_device::device_reset()
{
	// Clear address registers, counters and flags
	m_pc = 0;
	m_mb = 0;
	m_inst_disable = false;
	m_inst_repeat = false;
	set_bar(0);
	m_cntr = 0;
	m_sr = 0;
	m_spar = 0;
	m_flags = 0;
	m_unit = false;
	m_load_head = false;

	// Clear interface outputs (inactive high)
	for (auto &cb : m_interface_callback)
		cb(1);
}

void rx01_cpu_device::execute_set_input(int linenum, int state)
{
	// All inputs (and outputs) are active low
	switch (linenum)
	{
	case RX_RUN:
		m_run = (state == ASSERT_LINE);
		break;

	case RX_12_BIT:
		m_12_bit = (state == ASSERT_LINE);
		break;

	case RX_DATA:
		m_data_in = (state == ASSERT_LINE);
		break;
	}
}

u8 rx01_cpu_device::mux_out()
{
	if (BIT(m_mb, 0))
		return m_sp[m_spar];
	else
		return m_inst_cache.read_byte(m_pc);
}

bool rx01_cpu_device::data_in()
{
	if (m_data_in)
		return true;
	else if (m_flags & FF_IOB3)
	{
		if (m_flags & FF_IOB6)
			return bool(m_data_cache.read_byte(m_bar));
		else
			return BIT(m_sr, 7);
	}
	else
		return false;
}

bool rx01_cpu_device::sep_data()
{
	// TODO
	return false;
}

bool rx01_cpu_device::sep_clk()
{
	// TODO
	return false;
}

bool rx01_cpu_device::missing_clk()
{
	// TODO
	return false;
}

bool rx01_cpu_device::drv_sel_trk0()
{
	// TODO
	return false;
}

bool rx01_cpu_device::sec_buf_in()
{
	if (m_flags & FF_IOB0)
		return sep_data();
	else
		return data_in();
}

bool rx01_cpu_device::test_condition()
{
	switch (m_mb & 074)
	{
	case 000:
		// Interface transfer request or command pending
		return m_run;

	case 004:
		// Output buffer bit 3
		return (m_flags & FF_IOB3) != 0;

	case 010:
		// Serial data from interface
		return data_in();

	case 014:
		// Drive index latch
		return m_syn_index;

	case 020:
		// MSB of shift register
		return BIT(m_sr, 7);

	case 024:
		// Counter overflow
		return m_cntr == 0377;

	case 030:
		// 16th stage of CRC generator
		return BIT(m_crc, 0);

	case 034:
		// Track zero of selected drive on head
		return (m_flags & FF_IOB0) && (m_flags & FF_IOB3) && drv_sel_trk0();

	case 040:
		// Drive write protect (TODO)
		return false;

	case 044:
		// Separated clock
		return sep_clk();

	case 050:
		// 12-bit interface mode selected
		return m_12_bit;

	case 054:
		// Separated data equals shift register MSB
		return BIT(m_sr, 7) == sep_data();

	case 060:
		// Sector buffer address overflow
		return m_bar == 07777;

	case 064:
		// Missing clock equals shift register MSB
		return BIT(m_sr, 7) == missing_clk();

	case 070:
		// Sector buffer output
		if (m_flags & FF_WRTBUF)
			return sec_buf_in();
		else
			return bool(m_data_cache.read_byte(m_bar));

	case 074:
		// Flag state equals one
		return (m_flags & FF_FLAG) != 0;

	default:
		// Shouldn't happen
		logerror("%04o: Unhandled branch condition %d\n", m_ppc, (m_mb & 074) >> 2);
		return true;
	}
}

void rx01_cpu_device::set_bar(u16 bar)
{
	if (m_bar != bar && (m_flags & FF_WRTBUF))
		m_data_cache.write_byte(m_bar, sec_buf_in());
	m_bar = bar;
}

void rx01_cpu_device::shift_crc(bool data)
{
	// TODO: double-check algorithm
	if (data == BIT(m_crc, 0))
		m_crc = (m_crc >> 1) ^ 0002010;
	else
		m_crc = (m_crc >> 1) | 0100000;
}

void rx01_cpu_device::set_flag(bool j, bool k)
{
	if (j && !(m_flags & FF_FLAG))
		m_flags |= FF_FLAG;
	else if (k && (m_flags & FF_FLAG))
		m_flags &= ~FF_FLAG;
}

void rx01_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_inst_disable)
		{
			if ((m_mb & 0302) == 0202)
				m_pc = u16(m_mb & 074) << 6 | mux_out();
			else if (BIT(m_mb, 6) && m_br_condition)
				m_pc = ((m_pc + 1) & 07400) | mux_out();
			else
				m_pc = (m_pc + 1) & 07777;

			m_inst_disable = false;
			m_inst_repeat = false;
		}
		else
		{
			if (!m_inst_repeat)
			{
				m_ppc = m_pc;
				debugger_instruction_hook(m_pc);

				m_mb = m_inst_cache.read_byte(m_pc);
				m_pc = (m_pc + 1) & 03777;
			}

			if (BIT(m_mb, 6))
			{
				m_br_condition = test_condition() == BIT(m_mb, 1);
				if (BIT(m_mb, 7))
				{
					m_inst_disable = m_cntr == 0377 || m_br_condition;
					m_inst_repeat = m_cntr != 0377 && !m_br_condition;
					m_cntr++;
				}
				else
					m_inst_disable = true;
			}
			else if (BIT(m_mb, 7))
			{
				if (BIT(m_mb, 1))
					m_inst_disable = true;
				else
					m_spar = (m_mb & 074) >> 2;
			}
			else switch (m_mb & 074)
			{
			case 000:
				if (BIT(m_mb, 1) && (m_flags & FF_IOB0) == 0)
				{
					LOG("%04o: Drive bus selected\n", m_ppc);
					m_flags |= FF_IOB0;
					for (int i = 0; i < 5; i++)
						if (m_flags & (FF_IOB1 << i))
							m_interface_callback[i](1);
				}
				else if (!BIT(m_mb, 1) && (m_flags & FF_IOB0) != 0)
				{
					LOG("%04o: Interface bus selected\n", m_ppc);
					m_flags &= ~FF_IOB0;
					for (int i = 0; i < 5; i++)
						if (m_flags & (FF_IOB1 << i))
							m_interface_callback[i](0);
				}
				break;

			case 004:
				if (BIT(m_mb, 1) && (m_flags & FF_IOB1) == 0)
				{
					m_flags |= FF_IOB1;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX ERROR asserted\n", m_ppc);
						m_interface_callback[0](0);
					}
				}
				else if (!BIT(m_mb, 1) && (m_flags & FF_IOB1) != 0)
				{
					m_flags &= ~FF_IOB1;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX ERROR cleared\n", m_ppc);
						m_interface_callback[0](1);
					}
				}
				break;

			case 010:
				if (BIT(m_mb, 1) && (m_flags & FF_IOB2) == 0)
				{
					m_flags |= FF_IOB2;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX TRANSFER REQUEST asserted\n", m_ppc);
						m_interface_callback[1](0);
					}
				}
				else if (!BIT(m_mb, 1) && (m_flags & FF_IOB2) != 0)
				{
					m_flags &= ~FF_IOB2;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX TRANSFER REQUEST cleared\n", m_ppc);
						m_interface_callback[1](1);
					}
				}
				break;

			case 014:
				if (BIT(m_mb, 1) && (m_flags & FF_IOB3) == 0)
				{
					m_flags |= FF_IOB3;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX OUT mode selected\n", m_ppc);
						m_interface_callback[2](0);
					}
				}
				else if (!BIT(m_mb, 1) && (m_flags & FF_IOB3) != 0)
				{
					m_flags &= ~FF_IOB3;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX IN mode selected\n", m_ppc);
						m_interface_callback[2](1);
					}
				}
				break;

			case 020:
				if (BIT(m_mb, 1) && (m_flags & FF_IOB4) == 0)
				{
					m_flags |= FF_IOB4;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX DONE asserted\n", m_ppc);
						m_interface_callback[3](0);
					}
				}
				else if (!BIT(m_mb, 1) && (m_flags & FF_IOB4) != 0)
				{
					m_flags &= ~FF_IOB4;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX DONE cleared\n", m_ppc);
						m_interface_callback[3](1);
					}
				}
				break;

			case 024:
				if (BIT(m_mb, 1) && (m_flags & FF_IOB5) == 0)
				{
					m_flags |= FF_IOB5;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX SHIFT asserted\n", m_ppc);
						m_interface_callback[4](0);
					}
				}
				else if (!BIT(m_mb, 1) && (m_flags & FF_IOB5) != 0)
				{
					m_flags &= ~FF_IOB5;
					if ((m_flags & FF_IOB0) == 0)
					{
						LOG("%04o: RX SHIFT cleared\n", m_ppc);
						m_interface_callback[4](1);
					}
				}
				break;

			case 030:
				if (BIT(m_mb, 1))
				{
					LOG("%04o: SEC BUF selected for output\n", m_ppc);
					m_flags |= FF_IOB6;
				}
				else
				{
					LOG("%04o: SR selected for output\n", m_ppc);
					m_flags &= ~FF_IOB6;
				}
				break;

			case 034:
				m_unit = BIT(m_mb, 1);
				break;

			case 040:
				m_load_head = BIT(m_mb, 1);
				m_syn_index = false;
				break;

			case 044:
				if (BIT(m_mb, 1))
					set_bar((m_bar + 1) & 07777);
				else
					set_bar(BIT(m_mb, 0) ? 0 : 06000);
				break;

			case 050:
				if (BIT(m_mb, 0))
					m_flags |= FF_WRTBUF;
				else if (m_flags & FF_WRTBUF)
				{
					m_data_cache.write_byte(m_bar, sec_buf_in());
					m_flags &= ~FF_WRTBUF;
				}
				break;

			case 054:
				if ((m_mb & 3) == 3)
					m_crc = 0177777;
				else if (BIT(m_mb, 0))
					shift_crc(sep_data());
				else
					shift_crc(BIT(m_mb, 1));
				break;

			case 060:
				set_flag(BIT(m_mb, 1), BIT(m_mb, 0));
				break;

			case 064:
				m_sp[m_spar] = m_sr;
				break;

			case 070:
				if (BIT(m_mb, 1))
					m_cntr++;
				else
					m_cntr = mux_out();
				m_inst_disable = !BIT(m_mb, 0);
				break;

			case 074:
				if ((m_mb & 3) == 1)
					m_sr = m_cntr;
				else if (BIT(m_mb, 0))
					m_sr = (m_sr << 1) | sep_data();
				else
					m_sr = (m_sr << 1) | BIT(m_mb, 1);
				break;
			}
		}

		m_icount--;
	}
}

void rx01_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		if (m_flags & FF_IOB0)
			str = string_format("D%c%c%c%c%c%c %4s",
								(m_flags & FF_WRTBUF) ? 'B' : ':',
								(m_flags & FF_IOB1) ? 'W' : '.',
								(m_flags & FF_IOB2) ? 'S' : '.',
								(m_flags & FF_IOB3) ? 'H' : '.',
								(m_flags & FF_IOB4) ? 'E' : '.',
								(m_flags & FF_IOB5) ? 'T' : '.',
								(m_flags & FF_FLAG) ? ((m_flags & FF_IOB1) ? "DATA" : "FLAG") : "");
		else
			str = string_format("I%c%c%c%c%c%c %s %c",
								(m_flags & FF_WRTBUF) ? 'B' : ':',
								(m_flags & FF_IOB1) ? 'E' : '.',
								(m_flags & FF_IOB2) ? 'R' : '.',
								(m_flags & FF_IOB3) ? 'O' : '.',
								(m_flags & FF_IOB4) ? 'D' : '.',
								(m_flags & FF_IOB5) ? 'S' : '.',
								(m_flags & FF_IOB6) ? "SB" : "SR",
								(m_flags & FF_FLAG) ? 'F' : ' ');
		break;

	case RX01_SPAR:
		str = string_format("R%-2d", m_spar);
		break;
	}
}
