// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Intel 82750PB pixel processor

    Functional interpreter based on Intel order 240873-001.

    TODO:
    - 82750PA emulation mode
    - VRAM arbitration, FIFO buffering and host-access freezes
    - exact pixel interpolator rounding
*/

#include "emu.h"
#include "i750.h"
#include "i750d.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(I82750PB, i82750pb_device, "i82750pb", "Intel 82750PB Pixel Processor")

i82750pb_device::i82750pb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, I82750PB, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 64, 9, -3, address_map_constructor(FUNC(i82750pb_device::program_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 32, 32)
	, m_ram_config("internal", ENDIANNESS_LITTLE, 16, 9, -1, address_map_constructor(FUNC(i82750pb_device::ram_map), this))
	, m_code(*this, "microcode")
	, m_ram(*this, "ram")
	, m_irq_cb(*this)
	, m_reg{}
	, m_old_ptr{}
	, m_pointer{}
	, m_pc(0)
	, m_a(0)
	, m_b(0)
	, m_alu(0)
	, m_cc(0)
	, m_bus_b(0)
	, m_pending_pc(-1)
	, m_lc(0)
	, m_prof(false)
	, m_irq(false)
	, m_input{}
	, m_output{}
	, m_stat_table{}
	, m_stat_index(0)
	, m_stat_bit(0)
	, m_stat_buffer(0)
	, m_stat_valid(false)
	, m_pixels{}
	, m_pixel_head(0)
	, m_pixel_count(0)
	, m_pixel_inputs(0)
	, m_pixel_top(0)
	, m_pixel_prev_top(0)
	, m_pixel_prev_bottom(0)
	, m_pixel_previous(false)
	, m_icount(0)
{
}

void i82750pb_device::program_map(address_map &map)
{
	map(0x000, 0x1ff).ram().share("microcode");
}

void i82750pb_device::ram_map(address_map &map)
{
	map(0x000, 0x1ff).ram().share("ram");
}

device_memory_interface::space_config_vector i82750pb_device::memory_space_config() const
{
	return {{AS_PROGRAM, &m_program_config}, {AS_DATA, &m_data_config}, {AS_IO, &m_ram_config}};
}

std::unique_ptr<util::disasm_interface> i82750pb_device::create_disassembler()
{
	return std::make_unique<i750_disassembler>();
}

u32 i82750pb_device::execute_min_cycles() const noexcept
{
	return 1;
}

u32 i82750pb_device::execute_max_cycles() const noexcept
{
	return 2;
}

void i82750pb_device::device_start()
{
	space(AS_DATA).specific(m_data);
	set_icountptr(m_icount);

	state_add(STATE_GENPC, "PC", m_pc).mask(0x1ff);
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "CC", m_cc).mask(0x1e);
	for (unsigned i = 0; i < 16; i++)
	{
		state_add(i, string_format("R%d", i).c_str(), m_reg[R0 + i]);
	}
	state_add(16, "CONTROL", m_control);
	state_add(17, "ALU", m_alu);
	state_add(18, "STATBIT", m_stat_bit);
	state_add(19, "IN1", m_input[0]);
	state_add(20, "IN2", m_input[1]);
	state_add(21, "OUT1", m_output[0]);
	state_add(22, "OUT2", m_output[1]);

	save_item(NAME(m_reg));
	save_item(NAME(m_pointer));
	save_item(NAME(m_pc));
	save_item(NAME(m_pending_pc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_alu));
	save_item(NAME(m_cc));
	save_item(NAME(m_lc));
	save_item(NAME(m_bus_b));
	save_item(NAME(m_irq));
	save_item(NAME(m_control));
	save_item(NAME(m_interrupt));
	save_item(NAME(m_prof));
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_stat_table));
	save_item(NAME(m_stat_index));
	save_item(NAME(m_stat_bit));
	save_item(NAME(m_stat_buffer));
	save_item(NAME(m_stat_valid));
	save_item(NAME(m_pixels));
	save_item(NAME(m_pixel_head));
	save_item(NAME(m_pixel_count));
	save_item(NAME(m_pixel_inputs));
	save_item(NAME(m_pixel_top));
	save_item(NAME(m_pixel_prev_top));
	save_item(NAME(m_pixel_prev_bottom));
	save_item(NAME(m_pixel_previous));
}

void i82750pb_device::device_reset()
{
	// halted in 82750PA mode, with all interrupts masked and cleared
	m_control = 1;
	m_interrupt = 0;
	m_pointer[25] = 0x3fff; // 14 refresh cycles per RFSH code
	update_irq();
}

i82750pb_device::reg_id i82750pb_device::a_source(unsigned code)
{
	static constexpr reg_id SOURCES[64] =
	{
		NUL, HWID, CC, RESERVED, ALU, CNT, CNT2, LCNT,
		R0, R1, R2, R3, R4, R5, R6, R7,
		MCODE3, MCODE2, MCODE1, PC, RESERVED, PIXINT, RAM1, RAM2,
		RAM1_INC, RAM2_INC, RAM1_DEC, RAM2_DEC, DRAM1, DRAM2, DRAM3, DRAM4,
		IN1, IN2, STAT, STAT_STOP, RESERVED, RESERVED, RESERVED, RESERVED,
		RESERVED, RESERVED, RESERVED, IN1_HI, RESERVED, RESERVED, RESERVED, IN2_HI,
		R8, R9, R10, R11, R12, R13, R14, R15,
		SHIFT, RESERVED, RAM3, RAM4, RAM3_INC, RAM4_INC, RAM3_DEC, RAM4_DEC,
	};

	return SOURCES[code & 63];
}

i82750pb_device::reg_id i82750pb_device::a_destination(unsigned code)
{
	static constexpr reg_id DESTINATIONS[64] =
	{
		NUL, RESERVED, RESERVED, MADDR, RESERVED, CNT, CNT2, RESERVED,
		R0, R1, R2, R3, R4, R5, R6, R7,
		MCODE3, MCODE2, MCODE1, PC, PIXINT_C, PIXINT, RAM1, RAM2,
		RAM1_INC, RAM2_INC, RAM1_DEC, RAM2_DEC, DRAM1, DRAM2, DRAM3, DRAM4,
		OUT1, OUT1_INC, SHIFT_RL, OUT1_HI, OUT2, OUT2_INC, SHIFT_R, OUT2_HI,
		OUT1_C, IN1_C, SHIFT_L, IN1_HI, OUT2_C, IN2_C, RESERVED, IN2_HI,
		R8, R9, R10, R11, R12, R13, R14, R15,
		CC, RESERVED, RAM3, RAM4, RAM3_INC, RAM4_INC, RAM3_DEC, RAM4_DEC,
	};

	return DESTINATIONS[code & 63];
}

i82750pb_device::reg_id i82750pb_device::b_source(unsigned code)
{
	static constexpr reg_id SOURCES[64] =
	{
		NUL, ALU, RAM3, RAM4, RAM3_INC, RAM4_INC, RAM3_DEC, RAM4_DEC,
		R0, R1, R2, R3, R4, R5, R6, R7,
		IN1, IN2, STAT, STAT_STOP, RESERVED, RESERVED, RESERVED, RESERVED,
		IMM0, IMM1, IMM2, IMM3, IMM4, IMM5, IMM6, IMM7,
		PROF, RESERVED, OUT1_LO, OUT1_HI, STAT_LO, STAT_HI, OUT2_LO, OUT2_HI,
		OUT1_C, IN1_C, IN1_LO, IN1_HI, OUT2_C, IN2_C, IN2_LO, IN2_HI,
		R8, R9, R10, R11, R12, R13, R14, R15,
		SHIFT, STAT_C, RAM1, RAM2, RAM1_INC, RAM2_INC, RAM1_DEC, RAM2_DEC,
	};

	return SOURCES[code & 63];
}

i82750pb_device::reg_id i82750pb_device::b_destination(unsigned code)
{
	static constexpr reg_id DESTINATIONS[64] =
	{
		NUL, RESERVED, RAM3, RAM4, RAM3_INC, RAM4_INC, RAM3_DEC, RAM4_DEC,
		R0, R1, R2, R3, R4, R5, R6, R7,
		R8, R9, R10, R11, R12, R13, R14, R15,
		RESERVED, RESERVED, RAM1, RAM2, RAM1_INC, RAM2_INC, RAM1_DEC, RAM2_DEC,
		OUT1, OUT1_INC, OUT1_LO, OUT1_HI, OUT2, OUT2_INC, OUT2_LO, OUT2_HI,
		OUT1_C, IN1_C, IN1_LO, IN1_HI, OUT2_C, IN2_C, IN2_LO, IN2_HI,
		STAT_RAM, STAT_C, STAT_LO, STAT_HI, YEVEN_LO, YEVEN_HI, YODD_LO, YODD_HI,
		YPITCH, RESERVED, VU_LO, VU_HI, VUPITCH, VPITCH, VPTR_LO, VPTR_HI,
	};

	return DESTINATIONS[code & 63];
}

i82750pb_device::reg_id i82750pb_device::pointer_register(unsigned index)
{
	// pointer RAM slots that mirror a register
	static constexpr reg_id REGISTERS[32] =
	{
		RESERVED, RESERVED, OUT1_LO, OUT1_HI, RESERVED, RESERVED, OUT2_LO, OUT2_HI,
		RESERVED, RESERVED, IN1_LO, IN1_HI, RESERVED, RESERVED, IN2_LO, IN2_HI,
		RESERVED, RESERVED, RESERVED, RESERVED, YEVEN_LO, YEVEN_HI, YODD_LO, YODD_HI,
		YPITCH, RESERVED, VU_LO, VU_HI, VUPITCH, VPITCH, VPTR_LO, VPTR_HI
	};

	return REGISTERS[index & 31];
}

void i82750pb_device::snapshot_pointers()
{
	for (unsigned i = 0; i < 4; i++)
	{
		m_old_ptr[i] = m_reg[DRAM1 + i] & 0x1ff;
	}
}

unsigned i82750pb_device::ram_address(reg_id reg, bool effects)
{
	const unsigned index = (reg - RAM1) / 3;
	const unsigned mode = (reg - RAM1) % 3;
	const u16 address = m_old_ptr[index];
	if (effects && mode)
	{
		m_reg[DRAM1 + index] = (address + ((mode == 1) ? 1 : -1)) & 0x1ff;
	}
	return address;
}

u16 i82750pb_device::source(reg_id reg, bool effects)
{
	if (reg >= RAM1 && reg <= RAM4_DEC)
	{
		return m_ram[ram_address(reg, effects)];
	}
	if (reg >= IMM0 && reg <= IMM7)
	{
		return reg - IMM0;
	}

	switch (reg)
	{
		case NUL:
		case RESERVED:
			return 0xffff; // undriven bus

		case HWID:
			return SIGNATURE;

		case ALU:
			return m_alu;

		case CC:
			return m_cc | ((m_reg[m_lc ? CNT2 : CNT] == 0) ? 0x20 : 0) | (BIT(m_reg[R0], 0) << 6) | (BIT(m_reg[R0], 15) << 7);

		case PC:
			return m_pc;

		case MCODE1:
			return m_code[m_pc] >> 32;

		case MCODE2:
			return m_code[m_pc] >> 16;

		case MCODE3:
			return m_code[m_pc];

		case IN1:
		case IN2:
		{
			const unsigned i = (reg == IN2) ? 1 : 0;
			const u16 ctl = m_reg[i ? IN2_C : IN1_C];
			const unsigned n = BIT(ctl, 0) ? 1 : 2;
			const u16 value = (n == 1) ? m_data.read_byte(m_input[i]) : m_data.read_word(m_input[i]);
			if (effects)
			{
				m_input[i] += (BIT(ctl, 3) ? 4 : n) * (BIT(ctl, 1) ? -1 : 1);
			}
			return value;
		}

		case STAT:
		case STAT_STOP:
		{
			const bool table_access = (m_reg[STAT_C] & 0x30) != 0;
			if (!effects)
			{
				return (m_stat_valid && !table_access) ? m_stat_buffer : stat_decode(false);
			}
			if (table_access)
			{
				return stat_decode();
			}

			// STAT prefetches the next symbol, STAT_STOP doesn't
			const u16 value = m_stat_valid ? m_stat_buffer : stat_decode();
			m_stat_valid = (reg == STAT);
			if (m_stat_valid)
			{
				m_stat_buffer = stat_decode();
			}
			return value;
		}

		case PIXINT:
			return pixel_read(effects);

		case PROF:
			if (effects)
			{
				m_prof = !m_prof;
			}
			return 0;

		default:
			return m_reg[reg];
	}
}

void i82750pb_device::destination(reg_id reg, u16 value)
{
	if (reg >= RAM1 && reg <= RAM4_DEC)
	{
		m_ram[ram_address(reg, true)] = value;
		return;
	}
	if (reg >= DRAM1 && reg <= DRAM4)
	{
		value &= 0x1ff;
	}

	switch (reg)
	{
		case NUL:
		case RESERVED:
			return;

		case MADDR:
			m_code[value & 0x1ff] = (u64(m_reg[MCODE1]) << 32) | (u64(m_reg[MCODE2]) << 16) | m_reg[MCODE3];
			m_icount--;
			return;

		case PC:
			m_pending_pc = value & 0x1ff;
			return;

		case CC:
			m_cc = value & 0x1e;
			return;

		case OUT1:
		case OUT2:
		case OUT1_INC:
		case OUT2_INC:
		{
			const unsigned i = (reg == OUT2 || reg == OUT2_INC) ? 1 : 0;
			const u16 ctl = m_reg[i ? OUT2_C : OUT1_C];
			const unsigned n = BIT(ctl, 0) ? 1 : 2;
			const u32 address = m_output[i];
			if (reg == OUT1 || reg == OUT2)
			{
				if (BIT(ctl, 4))
				{
					value = (value & 0xfefe) | (BIT(ctl, 3) ? 0x0101 : 0);
				}
				if (n == 1)
				{
					m_data.write_byte(address, value);
				}
				else
				{
					m_data.write_word(address, value);
				}
			}

			u32 next = address + (BIT(ctl, 5) ? 4 : n) * (BIT(ctl, 1) ? -1 : 1);
			if (BIT(ctl, 2))
			{
				next = (address & ~7) | (next & 7);
			}
			m_output[i] = next;
			return;
		}

		case SHIFT_L:
			m_reg[SHIFT] = u32(m_bus_b) << (value & 15);
			return;

		case SHIFT_R:
			m_reg[SHIFT] = s16(m_bus_b) >> (value & 15);
			return;

		case SHIFT_RL:
			m_reg[SHIFT] = m_bus_b >> (value & 15);
			return;

		case STAT_C:
			if (value & 0x30)
			{
				m_stat_index = value & 7;
			}
			break;

		case STAT_RAM:
			m_stat_table[m_stat_index] = value;
			m_stat_index = (m_stat_index + 1) & 7;
			break;

		case STAT_LO:
			m_stat_bit = u64((u32(m_reg[STAT_HI]) << 16) | (value & ~3)) * 8;
			m_stat_valid = false;
			break;

		case IN1_LO:
			m_input[0] = (u32(m_reg[IN1_HI]) << 16) | value;
			break;

		case IN2_LO:
			m_input[1] = (u32(m_reg[IN2_HI]) << 16) | value;
			break;

		case OUT1_LO:
			m_output[0] = (u32(m_reg[OUT1_HI]) << 16) | value;
			break;

		case OUT1_HI:
			m_output[0] = (u32(value) << 16) | m_reg[OUT1_LO];
			break;

		case OUT2_LO:
			m_output[1] = (u32(m_reg[OUT2_HI]) << 16) | value;
			break;

		case OUT2_HI:
			m_output[1] = (u32(value) << 16) | m_reg[OUT2_LO];
			break;

		case PIXINT_C:
			if ((value | m_reg[PIXINT_C]) & 0x400)
			{
				m_pixel_head = m_pixel_count = m_pixel_inputs = 0;
				m_pixel_previous = false;
				if (value & 0x300)
				{
					const unsigned primed = (BIT(value, 14) ? 2 : 6) + BIT(value, 13);
					for (unsigned i = 0; i < primed; i++)
					{
						pixel_push(0);
					}
				}
			}
			break;

		case PIXINT:
			pixel_write(value);
			break;

		default:
			break;
	}

	m_reg[reg] = value;
	for (unsigned i = 20; i < 32; i++)
	{
		if (pointer_register(i) == reg)
		{
			m_pointer[i] = value;
		}
	}
}

u16 i82750pb_device::stat_decode(bool effects)
{
	const u16 ctl = m_reg[STAT_C];
	if (BIT(ctl, 5))
	{
		const u16 value = m_stat_table[m_stat_index];
		if (effects)
		{
			m_stat_index = (m_stat_index + 1) & 7;
		}
		return value;
	}
	if (BIT(ctl, 4))
	{
		return 0xffff;
	}

	u64 bit = m_stat_bit;
	const auto read_bit = [this, &bit] ()
	{
		const u64 current = bit++;
		return BIT(m_data.read_byte(u32(current >> 3)), current & 7);
	};

	u32 base = 0;
	for (unsigned index = 0; index < 65536; index++)
	{
		const u8 entry = m_stat_table[std::min(index, 7U)];
		const unsigned n = BIT(ctl, 7) ? BIT(ctl, 8, 5) : (entry & 127);
		unsigned bits = 0;
		for (unsigned v = n; v > 1; v >>= 1)
		{
			bits++;
		}

		const bool same = read_bit() == BIT(~ctl, 15);
		if (!same || (BIT(ctl, 6) && BIT(entry, 7)))
		{
			if (same)
			{
				base += std::max(1U, n);
			}

			unsigned tail = 0;
			while (bits--)
			{
				tail = (tail << 1) | read_bit();
			}
			if (effects)
			{
				m_stat_bit = bit;
			}
			return base + tail;
		}
		base += std::max(1U, n);
	}

	if (effects)
	{
		m_stat_bit = bit;
		logerror("Unterminated statistical symbol at %08x, PC %03x\n", u32(bit >> 3), m_pc);
	}
	return 0xffff;
}

void i82750pb_device::pixel_push(u8 data)
{
	if (m_pixel_count == std::size(m_pixels))
	{
		pixel_pop();
	}
	m_pixels[(m_pixel_head + m_pixel_count++) & 31] = data;
}

u8 i82750pb_device::pixel_pop()
{
	if (!m_pixel_count)
	{
		return 0;
	}

	m_pixel_count--;
	const u8 value = m_pixels[m_pixel_head];
	m_pixel_head = (m_pixel_head + 1) & 31;
	return value;
}

u16 i82750pb_device::pixel_read(bool effects)
{
	if (!m_pixel_previous)
	{
		return 0;
	}

	if (!effects)
	{
		u16 value = m_pixel_count ? m_pixels[m_pixel_head] : 0;
		if (BIT(m_reg[PIXINT_C], 11) && (m_pixel_count > 1))
		{
			value |= u16(m_pixels[(m_pixel_head + 1) & 31]) << 8;
		}
		return value;
	}

	u16 value = pixel_pop();
	if (BIT(m_reg[PIXINT_C], 11))
	{
		value |= u16(pixel_pop()) << 8;
	}
	return value;
}

void i82750pb_device::pixel_write(u16 data)
{
	if (!m_pixel_inputs++)
	{
		m_pixel_top = data;
		return;
	}

	const u16 ctl = m_reg[PIXINT_C];
	const unsigned h = ctl & 14;
	const unsigned v = (ctl >> 4) & 14;
	const auto interpolate = [h, v] (u8 a, u8 b, u8 c, u8 d) -> u8
	{
		// FIXME: intermediate rounding isn't documented
		return (a * (16 - h) * (16 - v) + b * h * (16 - v) + c * (16 - h) * v + d * h * v) >> 8;
	};

	if ((ctl & 0x300) && m_pixel_previous)
	{
		pixel_push(interpolate(m_pixel_prev_top >> 8, m_pixel_top, m_pixel_prev_bottom >> 8, data));
	}
	pixel_push(interpolate(m_pixel_top, m_pixel_top >> 8, data, data >> 8));

	m_pixel_prev_top = m_pixel_top;
	m_pixel_prev_bottom = data;
	m_pixel_previous = true;
	m_pixel_inputs = 0;
}

void i82750pb_device::alu(unsigned op, unsigned select, u16 av, u16 bv)
{
	if (select & 2)
	{
		m_a = av;
	}
	if (select & 1)
	{
		m_b = bv;
	}

	const u32 a = m_a;
	const u32 b = m_b;
	const unsigned carry = BIT(m_cc, 1);
	u32 value = 0;
	bool logical = false;
	switch (op)
	{
		case 0: return;
		case 1: m_alu = 0; return;
		case 2: value = a; logical = true; break;
		case 3: value = b; logical = true; break;
		case 4: value = ~a; logical = true; break;
		case 5: value = ~b; logical = true; break;
		case 6: value = a & b; logical = true; break;
		case 7: value = ~a & b; logical = true; break;
		case 8: value = a & ~b; logical = true; break;
		case 9: value = a + b + 1; break;
		case 10: value = a | b; logical = true; break;
		case 11: value = ~(a | b); logical = true; break;
		case 12: value = a ^ b; logical = true; break;
		case 13: value = a + (b ^ 0xffff) + carry; break;
		case 14: value = a + (b ^ 0xffff); break;
		case 15: value = b + (a ^ 0xffff) + carry; break;
		case 16: value = a + b; break;
		case 17: value = a + (b ^ 0xffff) + 1; break;
		case 18: value = b + (a ^ 0xffff) + 1; break;
		case 19: value = (a ^ 0xffff) + 1; break;
		case 20: value = (b ^ 0xffff) + 1; break;
		case 21: value = a + 1; break;
		case 22: value = b + 1; break;
		case 23: value = a + 0xffff; break;
		case 24: value = b + 0xffff; break;
		case 25: interrupt(0x2000); return;
		case 26: m_prof = !m_prof; return;
		case 27: m_alu = a; return;
		case 28: m_alu = b; return;
		case 29: value = a + b + carry; break;
		case 30:
		case 31:
			// packed bytes, clamped to 0-255
			for (const unsigned shift : { 0, 8 })
			{
				const int x = (a >> shift) & 255;
				const int y = (b >> shift) & 255;
				value |= std::clamp((op == 30) ? (x + y - 128) : (x - y + 128), 0, 255) << shift;
			}
			logical = true;
			break;
	}

	m_alu = value;
	const bool zero = (m_alu == 0);
	bool overflow;
	if (logical)
	{
		overflow = zero;
	}
	else if (op == 13 || op == 14 || op == 17)
	{
		overflow = (a ^ b) & (a ^ m_alu) & 0x8000;
	}
	else if (op == 15 || op == 18)
	{
		overflow = (a ^ b) & (b ^ m_alu) & 0x8000;
	}
	else if (op == 19)
	{
		overflow = (a == 0x8000);
	}
	else if (op == 20)
	{
		overflow = (b == 0x8000);
	}
	else if (op == 21 || op == 23)
	{
		overflow = (a == ((op == 21) ? 0x7fff : 0x8000));
	}
	else if (op == 22 || op == 24)
	{
		overflow = (b == ((op == 22) ? 0x7fff : 0x8000));
	}
	else
	{
		overflow = ~(a ^ b) & (a ^ m_alu) & 0x8000;
	}

	const bool carry_out = logical ? zero : BIT(value, 16);
	m_cc = (carry_out ? 0x02 : 0) | (overflow ? 0x04 : 0) | (BIT(m_alu, 15) << 3) | (zero ? 0x10 : 0);
}

void i82750pb_device::execute_run()
{
	while (m_icount > 0)
	{
		// halted, or in the unsupported 82750PA mode
		if (!BIT(m_control, 15) || (BIT(m_control, 0) && !BIT(m_control, 1)))
		{
			m_icount = 0;
			return;
		}

		debugger_instruction_hook(m_pc);
		const u64 w = m_code[m_pc];
		const bool literal = BIT(w, 37);
		const reg_id as = a_source(BIT(w, 12, 6));
		const reg_id ad = a_destination(BIT(w, 18, 6));
		const reg_id bs = b_source(BIT(w, 24, 6));
		const reg_id bd = b_destination(BIT(w, 30, 6));
		m_lc = BIT(w, 47);

		u16 next = w & 0x1ff;
		if (BIT(source(CC), literal ? 0 : BIT(w, 9, 3)))
		{
			next &= ~1;
		}

		const s16 pending = m_pending_pc;
		m_pending_pc = -1;
		snapshot_pointers();

		const u16 av = literal ? BIT(w, 9, 9) : source(as);
		const u16 bv = (!literal && (as == bs)) ? av : source(bs);
		m_bus_b = bv;
		destination(ad, av);
		destination(bd, bv);
		if (BIT(w, 36))
		{
			m_reg[m_lc ? CNT2 : CNT]--;
		}

		switch (BIT(w, 45, 2))
		{
			case 1:
				m_reg[R0] = s16(m_reg[R0]) >> 1;
				break;

			case 2:
				m_reg[R0] <<= 1;
				break;

			case 3:
				if (ad == R0 || bd == R0)
				{
					m_reg[R0] = swapendian_int16(m_reg[R0]);
				}
				break;
		}

		alu(BIT(w, 40, 5), BIT(w, 38, 2), av, bv);
		m_pc = (pending >= 0) ? pending : next;
		m_icount--;

		if (BIT(m_control, 1))
		{
			m_control = (m_control & ~2) | 1;
		}
	}
}

void i82750pb_device::update_irq()
{
	// the CONTROL enable bits are ordered differently from the INTERRUPT FLAG bits
	const u16 enable = (BIT(m_control, 2) << 13) | (BIT(m_control, 3) << 14) | (BIT(m_control, 4) << 15)
			| (BIT(m_control, 5) << 12) | (BIT(m_control, 6) << 10) | (BIT(m_control, 7) << 9);
	const bool irq = (m_interrupt & enable) != 0;
	if (m_irq != irq)
	{
		m_irq = irq;
		m_irq_cb(irq ? 1 : 0);
	}
}

void i82750pb_device::interrupt(u16 flags)
{
	// a second microcode interrupt before the host reads the flags sets the overflow flag
	if (flags & m_interrupt & 0x2000)
	{
		m_interrupt |= 0x800;
	}
	m_interrupt |= flags;
	update_irq();
}

u16 i82750pb_device::pointer_r(unsigned index) const
{
	index &= 31;
	const unsigned shift = BIT(index, 0) * 16;
	switch (index)
	{
		case 2:
		case 3:
			return m_output[0] >> shift;

		case 6:
		case 7:
			return m_output[1] >> shift;

		case 10:
		case 11:
			return m_input[0] >> shift;

		case 14:
		case 15:
			return m_input[1] >> shift;

		case 18:
		case 19:
			return (m_stat_bit >> 3) >> shift;

		default:
			return m_pointer[index];
	}
}

void i82750pb_device::pointer_w(unsigned index, u16 data)
{
	index &= 31;
	m_pointer[index] = data;
	const reg_id reg = pointer_register(index);
	if (reg != RESERVED)
	{
		destination(reg, data);
	}
}

void i82750pb_device::vblank_w(int state)
{
	if (state)
	{
		interrupt(0x4000);
	}
}

void i82750pb_device::db_shutdown_w(int state)
{
	if (state)
	{
		interrupt(0x1000);
	}
}

u16 i82750pb_device::host_r(offs_t offset)
{
	const bool effects = !machine().side_effects_disabled();
	offset &= 0xff;
	if (offset < 0x80)
	{
		snapshot_pointers();
		return source((offset < 0x40) ? a_source(offset) : b_source(offset), effects);
	}

	if (offset == 0x80)
	{
		// INTERRUPT FLAG, cleared by reading
		const u16 value = m_interrupt;
		if (effects)
		{
			m_interrupt = 0;
			update_irq();
		}
		return value;
	}
	if (offset == 0x81)
	{
		// PROCESSOR STATUS
		return (m_control & 1) | (m_prof ? 0x04 : 0) | ((m_control & 0xfc) << 8);
	}
	if (offset >= 0xc0)
	{
		return pointer_r(offset);
	}
	return 0xffff;
}

void i82750pb_device::host_w(offs_t offset, u16 data)
{
	offset &= 0xff;
	if (offset < 0x80)
	{
		snapshot_pointers();
		const reg_id reg = (offset < 0x40) ? a_destination(offset) : b_destination(offset);
		if (reg == PC)
		{
			// host PC loads take effect immediately
			m_pc = data & 0x1ff;
			m_pending_pc = -1;
		}
		else
		{
			destination(reg, data);
		}
	}
	else if (offset == 0x80)
	{
		m_control = data;
		update_irq();
	}
	else if (offset >= 0xc0)
	{
		pointer_w(offset, data);
	}
}
