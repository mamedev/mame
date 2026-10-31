// license:BSD-3-Clause
// copyright-holders:Eduardo Cruz
// TMS320C82 PP interpreter. Encoding and pipeline: TI SPRU110, chapters 6-11
// and appendix A; C82 reset stack layout: SPRU154 section 1.5.2, figure 1-5.
#include "emu.h"
#include "tms320c82.h"

#include <algorithm>
#include <bit>
#include <iterator>

namespace
{
uint32_t rotate(uint32_t v, uint32_t n)
{
	return std::rotl(v, int(n & 31));
}
uint32_t mask(uint32_t n)
{
	return util::make_bitmask<uint32_t>(n & 31);
}
uint32_t boolean(uint8_t f, uint32_t a, uint32_t b, uint32_t c)
{
	// Frequent move and mask operations need no truth-table expansion.
	switch (f)
	{
	case 0x00:
		return 0;
	case 0xff:
		return ~0U;
	case 0xaa:
		return a;
	case 0xcc:
		return b;
	case 0xf0:
		return c;
	case 0x88:
		return a & b;
	case 0xee:
		return a | b;
	case 0x44:
		return ~a & b;
	case 0x22:
		return a & ~b;
	case 0xa0:
		return a & c;
	case 0x0a:
		return a & ~c;
	case 0x50:
		return ~a & c;
	case 0xfa:
		return a | c;
	}
	uint32_t v = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(f, i))
			v |= (BIT(i, 0) ? a : ~a) & (BIT(i, 1) ? b : ~b) & (BIT(i, 2) ? c : ~c);
	}
	return v;
}

// Split ALU and expander lane width for a size code: 4 = 8 bits, 5 = 16 bits.
uint8_t lane_width(uint8_t size)
{
	return (size == 4) ? 8 : (size == 5) ? 16 : 32;
}

// SPRU110 13.1: the eight function bits select the propagate truth table.
// Carry generation, including the Boolean-with-carry EALUs, follows the same
// bit equations.
enum class c_input
{
	RAW,
	MASK,
	EXPAND
};
struct alu_result
{
	uint32_t value = 0, flags = 0, mf = 0;
};

alu_result extended_alu(
		uint32_t control, bool inverse, c_input mode,
		uint32_t source, uint32_t rotation, uint32_t a, uint32_t c,
		uint32_t sr, uint32_t mf)
{
	uint8_t const modifier = control >> 28;
	bool const arithmetic = BIT(control, 27), multiple = modifier >= 8;
	bool const detection = !multiple && BIT(modifier, 1) && mode != c_input::MASK;
	uint8_t const width = detection ? 8 : multiple ? lane_width(sr & 7) : 32;
	uint8_t const parts = 32 / width;
	uint8_t const expand_width = lane_width((sr >> 3) & 7);
	uint8_t const expand_parts = 32 / expand_width;
	bool const expanded = !detection && (mode == c_input::EXPAND || (mode == c_input::MASK && BIT(control, 13) && !(modifier < 8 && BIT(modifier, 1))));
	uint32_t const b = rotate(source, rotation);
	uint8_t function = ((control >> 19) & 255) ^ (inverse ? 255 : 0);
	bool const cin_modifier = !multiple && !detection && BIT(modifier, 0);
	bool const explicit_carry = arithmetic && BIT(control, 14) && !cin_modifier;
	bool carry_start = bool(BIT(control, 18)) != inverse;
	if (!explicit_carry && BIT(control, 16) && BIT(source, 31))
	{
		function ^= 15;
		if (BIT(control, 17))
			carry_start = !carry_start;
	}
	if (cin_modifier)
		carry_start = BIT(sr, 30);
	bool detection_empty = false;
	if (detection)
	{
		uint32_t input = source;
		if (BIT(modifier, 2) && BIT(input, BIT(modifier, 0) ? 0 : 31))
			input = ~input;
		detection_empty = input == 0;
		if (!input)
			c = 32;
		else
			c = BIT(modifier, 0) ? 31 - std::countr_zero(input) : std::countl_zero(input);
	}
	else
	{
		if (!multiple && BIT(modifier, 2))
			a = 0;
		if (expanded)
		{
			c = 0;
			for (int j = 0; j < expand_parts; j++)
			{
				if (BIT(mf, j + (BIT(sr, 6) ? 32 - expand_parts : 0)))
					c |= util::make_bitmask<uint32_t>(expand_width) << (j * expand_width);
			}
		}
		else if (mode == c_input::MASK)
		{
			uint8_t const mask_width = (multiple && !BIT(control, 15)) ? width : 32;
			uint8_t const n = c & (mask_width - 1);
			uint32_t lane = 0;
			if (n)
				lane = util::make_bitmask<uint32_t>(n);
			else if (modifier < 8 && BIT(modifier, 1))
				lane = util::make_bitmask<uint32_t>(mask_width);
			c = 0;
			for (int j = 0; j < 32; j += mask_width)
				c |= lane << j;
		}
	}
	uint32_t const propagate = boolean(function, a, b, c);
	uint32_t const generate = arithmetic ? a & ~propagate & boolean(function, 0, b, c) : 0;
	alu_result out;
	bool carry = false, last_cin = false, all_carry = true, any_carry = false, any_zero = false;
	uint32_t mf_bits = 0;
	for (int part = 0; part < parts; part++)
	{
		int const first = part * width;
		carry = explicit_carry ? BIT(c, first) != BIT(function, 0) : carry_start;
		for (int bit = first; bit < first + width; bit++)
		{
			last_cin = carry;
			out.value |= (BIT(propagate, bit) ^ uint32_t(carry)) << bit;
			carry = BIT(generate, bit) || (BIT(propagate, bit) && carry);
		}
		bool const zero = !BIT(out.value, first, width);
		any_zero |= zero;
		any_carry |= carry;
		all_carry &= carry;
		bool flag = carry;
		if (modifier & 1)
		{
			switch ((sr >> 7) & 3)
			{
			case 0:
				flag = zero;
				break;
			case 1:
				flag = BIT(out.value, first + width - 1);
				break;
			case 2:
				flag = BIT(propagate, first + width - 1) ^ carry;
				break;
			}
		}
		mf_bits |= uint32_t(flag) << part;
	}
	if (multiple || detection)
	{
		out.flags = (uint32_t(!out.value) << 31) | (uint32_t(any_carry) << 30)
				| (uint32_t(detection ? detection_empty : all_carry) << 29) | (uint32_t(any_zero) << 28);
	}
	else
	{
		out.flags = (out.value & 0x80000000U) | (uint32_t(carry) << 30)
				| (uint32_t(last_cin ^ carry) << 29) | (uint32_t(!out.value) << 28);
	}
	out.mf = mf;
	if (modifier == 10 || modifier == 11)
		out.mf = rotate(mf, parts);
	else if (modifier == 12 || modifier == 13)
		out.mf = 0;
	else if (expanded && BIT(sr, 6))
		out.mf = rotate(mf, expand_parts);
	if (multiple && modifier != 14)
		out.mf = (out.mf & ~util::make_bitmask<uint32_t>(parts)) | mf_bits;
	return out;
}

} // anonymous namespace

tms320c82_pp_device::decoded tms320c82_pp_device::decode(uint64_t op)
{
	decoded d;
	if (BIT(op, 63) && ((op >> 43) & 3) == 3)
	{
		d.dstbank = (op >> 39) & 15;
		d.srcbank = (op >> 36) & 7;
		d.condition = (op >> 32) & 15;
		d.conditional_dst = true;
		return d;
	}
	uint8_t const l = (op >> 37) & 3, g = (op >> 15) & 3;
	auto conditional = [&d, op] ()
	{
		d.condition = (op >> 32) & 15;
		d.conditional_src = BIT(op, 31);
		d.conditional_dst = BIT(op, 30);
		d.protect = ((op >> 25) & 15) << 28;
	};
	auto move = [&d, op] (bool field)
	{
		d.move_src = ((op >> 10) & 7) | ((field ? 4 : (op >> 6) & 15) << 3);
		d.move_dst = ((op >> 3) & 7) | (((op >> 18) & 15) << 3);
		if (field)
		{
			d.field_size = (op >> 7) & 3;
			d.field_item = (op >> 22) & 3;
			d.field_signed = BIT(op, 9);
			d.replicate = BIT(op, 6);
		}
	};
	if (!l && !g)
	{
		conditional();
		uint8_t const type = (op >> 13) & 3;
		if (type < 2)
		{
			move(type == 1);
			d.move_conditional = BIT(op, 29);
		}
		else
		{
			d.srcbank = (op >> 6) & 15;
			d.dstbank = (op >> 18) & 15;
		}
		return d;
	}
	if (l)
	{
		auto &t = d.tr[0];
		t.mode = (op >> 35) & 15;
		t.reg = ((op >> 32) & 7) | 0x20;
		t.le = BIT(op, 31) | (BIT(op, g ? 21 : 17) << 1);
		t.size = (op >> 29) & 3;
		t.scale = BIT(op, 28);
		t.ar = (op >> 25) & 7;
		t.index = op & 7;
		if (!g)
		{
			t.relative = (op >> 22) & 3;
			if (BIT(op, 24))
			{
				t.reg = ((op >> 32) & 7) | (((op >> 18) & 15) << 3);
				t.index = op & 0x7fff;
				if (!t.size && t.scale)
					t.index |= 0x8000;
			}
			else if (!BIT(op, 14))
			{
				move(BIT(op, 13));
				if (BIT(op, 13))
					t.relative = 0;
			}
			else
			{
				d.srcbank = (op >> 6) & 15;
				d.dstbank = (op >> 18) & 15;
			}
		}
		if (!t.size && t.scale && (g || !BIT(op, 24)))
			t.index |= 8;
		t.indexed = !BIT(t.mode, 1);
	}
	if (g)
	{
		auto &t = d.tr[1];
		t.mode = (op >> 13) & 15;
		t.reg = ((op >> 10) & 7) | (((op >> 18) & (l || BIT(op, 2) ? 7 : 15)) << 3);
		t.le = BIT(op, 9) | (BIT(op, 17) << 1);
		t.size = (op >> 7) & 3;
		t.scale = BIT(op, 6);
		t.ar = ((op >> 3) & 7) | 8;
		t.index = (op >> 22) & 7;
		if (!l)
		{
			t.relative = op & 3;
			if (BIT(op, 2))
			{
				conditional();
				t.conditional = BIT(op, 29);
			}
			else
			{
				t.index = (op >> 22) & 0x7fff;
				if (!t.size && t.scale)
					t.index |= 0x8000;
			}
		}
		if (!t.size && t.scale && (l || BIT(op, 2)))
			t.index |= 8;
		t.indexed = !BIT(t.mode, 1);
	}
	return d;
}

const tms320c82_pp_device::decoded &tms320c82_pp_device::decode_cached(uint32_t pc, uint64_t op)
{
	auto &entry = m_decode_cache[(pc >> 3) & (m_decode_cache.size() - 1)];
	if (!entry.valid || entry.op != op)
	{
		entry.op = op;
		entry.value = decode(op);
		entry.valid = true;
	}
	return entry.value;
}

uint32_t tms320c82_pp_device::reg_r(uint8_t reg) const
{
	if (reg == 7 || reg == 15)
		return 0;
	if (reg == 14)
		reg = 6;
	if (reg == 0x38)
		return (m_pipe_pc[2] & ~7U) | (m_fetchpc & 3);
	if (reg == 0x39)
		return m_pipe_pc[1];
	if (reg == 0x3a)
		return m_pipe_pc[0];
	if (reg == 0x3e)
		return (m_master->packet_status(m_pp_number) << 28) | m_pp_number;
	return m_regs[reg];
}

void tms320c82_pp_device::reg_w(uint8_t reg, uint32_t data)
{
	if (reg == 7 || reg == 15 || reg == 0x3a || reg >= 0x7c)
		return;
	if (reg == 14)
		reg = 6;
	if (reg == 6)
		data &= ~3U;
	if (reg == 0x38 || reg == 0x39)
	{
		if (reg == 0x38)
			m_regs[0x3b] = (m_pipe_pc[2] + 8) & ~7U;
		// Ordinary writes to CALL/BR preserve the G and L control bits.
		m_fetchpc = (data & ~7U) | (m_fetchpc & 3);
		return;
	}
	if (reg == 0x29)
		data = (data & ~m_protect) | (m_regs[0x29] & m_protect);
	if (reg == 0x3d)
	{
		m_poll_safe = false;
		if (BIT(m_regs[0x3c], 0))
			m_regs[reg] |= data;
		else
			m_regs[reg] &= ~data;
		return;
	}
	if (reg == 0x3e)
	{
		m_poll_safe = false;
		m_regs[reg] = (data & 0xc0000000) | m_pp_number;
		m_master->packet_control(m_pp_number, data >> 28);
		return;
	}
	if ((reg >= 0x64 && reg <= 0x66) || (reg >= 0x68 && reg <= 0x6a) || (reg >= 0x6c && reg <= 0x6e))
	{
		uint8_t const n = reg & 3;
		m_regs[0x60 + n] = m_regs[0x64 + n] = data;
		if (reg >= 0x68)
		{
			m_regs[0x70 + n] = m_pipe_pc[2] + 8;
			if (reg < 0x6c)
				m_regs[0x74 + n] = m_pipe_pc[2] + 8;
			m_regs[0x3f] = (m_regs[0x3f] & ~(15U << (4 * n))) | ((9U + n) << (4 * n));
		}
		return;
	}
	m_regs[reg] = data;
}

bool tms320c82_pp_device::condition(uint8_t cond) const
{
	bool const n = BIT(m_regs[0x29], 31), c = BIT(m_regs[0x29], 30), v = BIT(m_regs[0x29], 29), z = BIT(m_regs[0x29], 28);
	switch (cond)
	{
	case 0:
		return true;
	case 1:
		return !n && !z;
	case 2:
		return !c || z;
	case 3:
		return c && !z;
	case 4:
		return n != v;
	case 5:
		return n != v || z;
	case 6:
		return n == v;
	case 7:
		return n == v && !z;
	case 8:
		return c;
	case 9:
		return !c;
	case 10:
		return z;
	case 11:
		return !z;
	case 12:
		return v;
	case 13:
		return !v;
	case 14:
		return n;
	case 15:
		return !n;
	}
	return false;
}

inline uint64_t tms320c82_pp_device::fetch(uint32_t address)
{
	address &= ~7U;
	return (uint64_t(m_cache.read_dword(address)) << 32) | m_cache.read_dword(address + 4);
}

// Port loads and stores of a byte, halfword or word; doubleword transfers
// aren't supported.
inline bool tms320c82_pp_device::load(transfer const &t, uint32_t address, uint32_t &value)
{
	switch (t.size)
	{
	case 0:
		value = m_program.read_byte(address);
		if (t.le == 3)
			value = util::sext(value, 8);
		return true;
	case 1:
		value = m_program.read_word(address & ~1U);
		if (t.le == 3)
			value = util::sext(value, 16);
		return true;
	case 2:
		value = m_program.read_dword(address & ~3U);
		return true;
	}
	return false;
}

inline bool tms320c82_pp_device::store(transfer const &t, uint32_t address, uint32_t value)
{
	switch (t.size)
	{
	case 0:
		m_program.write_byte(address, value);
		return true;
	case 1:
		m_program.write_word(address & ~1U, value);
		return true;
	case 2:
		m_program.write_dword(address & ~3U, value);
		return true;
	}
	return false;
}

// A local memory access must use the global port if it is not to this PP's
// RAM. With a parallel global operation it therefore takes a second cycle,
// after the global access (SPRU110 5.2, 6.3.2). Accesses to the same local
// RAM bank also contend.
inline bool tms320c82_pp_device::local_diverted(decoded const &d) const
{
	auto const &local = d.tr[0], &global = d.tr[1];
	uint32_t const local_bank = m_ea[0] & ~0xfffU;
	uint32_t const pp_base = m_pp_number << 12;
	bool const local_ram = local_bank == pp_base || local_bank == (0x8000 | pp_base) || local_bank == (0x01000000 | pp_base);
	bool const same_bank = global.mode && global.le != 1 && local_bank == (m_ea[1] & ~0xfffU);
	return local.mode && local.le != 1 && (global.mode || d.move_src >= 0) && (!local_ram || same_bank);
}

bool tms320c82_pp_device::unsupported(uint64_t op, const char *reason)
{
	logerror("unimplemented %s at %08x: %016x\n", reason, m_pipe_pc[0] & ~7U, op);
	// TODO: implement the remaining instructions and exception behavior.
	m_halted = true;
	m_icount = 0;
	return false;
}

bool tms320c82_pp_device::execute_one(uint64_t op, const decoded &d)
{
	bool const test = condition(d.condition);
	uint32_t const old_sr = m_regs[0x29];
	m_protect = d.protect;
	// Capture both transfer operands before either unit writes a register.
	uint32_t transferred[2]{};
	for (int i = 0; i < 2; i++)
	{
		auto const &t = d.tr[i];
		if (!t.mode)
			continue;
		// Only direct RAM/ROM reads keep an idle loop skippable.
		if (m_poll_safe && (!t.le || (t.le >= 2 && !m_master->space(AS_PROGRAM).get_read_ptr(m_ea[i]))))
			m_poll_safe = false;
		uint8_t src = t.reg;
		if (!t.le && t.conditional && !test)
			src &= ~1;
		if (m_halt_transfer_ready || (m_transfer_pending && (i == 1 || !t.le)))
			transferred[i] = m_transfer_value[i];
		else if (!t.le)
			transferred[i] = reg_r(src);
		else if (t.le == 1)
			transferred[i] = m_ea[i];
		else if (!load(t, m_ea[i], transferred[i]))
			return unsupported(op, "transfer size");
	}
	uint32_t moved = d.move_src >= 0 ? ((m_halt_transfer_ready || m_transfer_pending) ? m_transfer_value[1] : reg_r(d.move_src)) : 0;
	if (d.field_size >= 0)
	{
		if (d.field_size > 1)
			return unsupported(op, "field size");
		uint8_t const bits = 8 << d.field_size;
		if (d.replicate)
		{
			moved = (bits == 8) ? (moved & 0xff) * 0x01010101U : (moved & 0xffff) * 0x00010001U;
		}
		else
		{
			moved >>= (d.field_item * bits) & 31;
			moved = d.field_signed ? util::sext(moved, bits) : moved & util::make_bitmask<uint32_t>(bits);
		}
	}
	if (!d.conditional_dst || test)
	{
		uint8_t s1 = ((op >> 45) & 7) | (d.srcbank << 3);
		if (d.conditional_src && !test)
			s1 &= ~1;
		uint8_t const dst = ((op >> 48) & 7) | (d.dstbank << 3);
		uint32_t const source = reg_r(s1);
		uint32_t a = 0, b = 0, c = 0, result = 0;
		uint64_t sum = 0;
		bool arithmetic = false, carry = false, overflow = false, do_alu = true;
		bool custom_flags = false;
		uint32_t flags = 0;
		int8_t mul_dst = -1;
		uint32_t product = 0;
		if ((op & 0xfaa8100000000000ULL) == 0x8800000000000000ULL)
		{
			do_alu = false;
			uint8_t const special = (op >> 39) & 31;
			if (special && special != 2 && special != 3)
				return unsupported(op, "special operation");
		}
		else if ((op >> 60) == 6 || (op >> 60) == 7)
		{
			uint8_t const type = (op >> 57) & 15;
			uint32_t const s3 = reg_r(0x20 | ((op >> 54) & 7));
			uint32_t const s4 = reg_r(0x20 | ((op >> 42) & 7));
			uint32_t const control = reg_r(0x20);
			a = reg_r(0x20 | ((op >> 39) & 7));
			mul_dst = 0x20 | ((op >> 51) & 7);
			if (type == 10)
			{
				// DIVI: shifted partial remainder and quotient use the old MF.
				product = (source << 1) | BIT(m_regs[0x2a], 31);
				sum = uint64_t(a) + product;
				result = sum;
				carry = BIT(sum, 32);
				bool const quotient = carry || BIT(source, 31);
				overflow = BIT(~(a ^ product) & (a ^ result), 31);
				flags = (uint32_t(quotient) << 31) | (uint32_t(carry) << 30)
						| (uint32_t(overflow) << 29) | (uint32_t(!result) << 28);
				m_regs[0x2a] = (m_regs[0x2a] << 1) | uint32_t(quotient);
				custom_flags = true;
				if (dst == mul_dst)
					mul_dst = -1;
			}
			else if (type == 8 || type == 9 || type == 11 || (type < 8 && BIT(type, 1)))
			{
				bool const multiply = type < 8;
				if (multiply)
				{
					bool const unsign = BIT(type, 2);
					if (BIT(control, 7))
					{
						if (BIT(control, 6))
							return unsupported(op, "reserved rounded split multiply");
						uint32_t const lo = uint8_t(s3) * (unsign ? int32_t(uint8_t(s4)) : int32_t(int8_t(s4)));
						uint32_t const hi = uint8_t(s3 >> 8) * (unsign ? int32_t(uint8_t(s4 >> 8)) : int32_t(int8_t(s4 >> 8)));
						uint8_t const shift = BIT(control, 8, 2);
						product = ((lo << shift) & 0xffff) | (hi << (16 + shift));
					}
					else
					{
						product = unsign ? uint32_t(uint16_t(s3)) * uint16_t(s4) : int32_t(int16_t(s3)) * int16_t(s4);
						product <<= (control >> 8) & 3;
						if (BIT(control, 6))
							product = ((product + 0x8000U) & 0xffff0000U) | (s3 >> 16);
					}
					if (dst == mul_dst)
						do_alu = false;
				}
				else
				{
					product = rotate(source, s3);
					if (dst == mul_dst)
						mul_dst = -1;
				}
				if (do_alu)
				{
					uint8_t const modifier = control >> 28;
					if (modifier == 15 || (modifier >= 8 && ((old_sr & 7) < 4 || (old_sr & 7) > 6)))
						return unsupported(op, "reserved EALU modifier/size");
					auto const r = extended_alu(
							control, BIT(type, multiply ? 0 : 1), (multiply || BIT(type, 0)) ? c_input::MASK : c_input::RAW,
							source, multiply ? control : s3, a, multiply ? control : s4, old_sr, m_regs[0x2a]);
					result = r.value;
					flags = r.flags;
					m_regs[0x2a] = r.mf;
					custom_flags = true;
				}
			}
			else
			{
				bool const unsign = type >= 12 ? BIT(type, 1) : BIT(type, 2);
				product = unsign ? uint32_t(uint16_t(s3)) * uint16_t(s4) : int32_t(int16_t(s3)) * int16_t(s4);
				b = source;
				if (type >= 12)
				{
					uint8_t const shift = -control & 31;
					b = unsign ? source >> shift : uint32_t(int32_t(source) >> shift);
				}
				bool const sub = BIT(type, 0);
				c = sub ? ~b : b;
				sum = uint64_t(a) + c + (sub ? 1 : 0);
				result = sum;
				carry = BIT(sum, 32);
				overflow = BIT((~(a ^ c) & (a ^ result)), 31);
				if (dst == mul_dst)
					do_alu = false;
			}
		}
		else if (BIT(op, 63))
		{
			uint8_t const format = (op >> 43) & 3, cl = (op >> 60) & 7, alu = (op >> 51) & 255;
			uint32_t const src2 = (format == 3) ? uint32_t(op) : (format == 2) ? reg_r(0x20 | BIT(op, 39, 3)) : uint32_t(BIT(op, 39, 5));
			uint32_t const d0 = reg_r(0x20), companion = reg_r(0x20 | ((op >> 48) & 7));
			uint8_t const shift = ((cl == 1 || cl == 4 || cl == 5) ? d0 : src2) & 31;
			bool const masked = cl == 2 || cl == 3 || cl == 4;
			uint32_t expanded = 0;
			uint8_t const width = lane_width((old_sr >> 3) & 7);
			uint8_t const count = 32 / width;
			if (cl == 0 || cl == 5)
			{
				for (int j = 0; j < count; j++)
				{
					if (BIT(m_regs[0x2a], j + (BIT(old_sr, 6) ? 32 - count : 0)))
						expanded |= util::make_bitmask<uint32_t>(width) << (j * width);
				}
			}
			switch (cl)
			{
			case 0:
				a = src2;
				b = source;
				c = expanded;
				break;
			case 1:
				a = companion;
				b = rotate(source, shift);
				c = src2;
				break;
			case 2:
				a = companion;
				b = source;
				c = mask(src2);
				break;
			case 3:
				a = companion;
				b = rotate(source, shift);
				c = mask(src2);
				break;
			case 4:
				a = src2;
				b = rotate(source, shift);
				c = mask(d0);
				break;
			case 5:
				a = src2;
				b = rotate(source, shift);
				c = expanded;
				break;
			case 6:
				a = companion;
				b = source;
				c = src2;
				break;
			case 7:
				a = source;
				b = 1U << (src2 & 31);
				c = src2;
				break;
			}
			arithmetic = BIT(op, 59);
			if (!arithmetic)
			{
				result = boolean(alu, a, b, c);
			}
			else
			{
				uint8_t const modifier = bitswap<4>(alu, 7, 5, 3, 1);
				uint8_t const function = bitswap<4>(alu, 6, 4, 2, 0);
				if ((modifier & 8) || ((modifier & 2) && !masked))
				{
					if (modifier == 15 || (modifier >= 8 && ((old_sr & 7) < 4 || (old_sr & 7) > 6)))
						return unsupported(op, "reserved ALU modifier/size");
					static constexpr uint8_t FUNCTIONS[16] = {0xaa, 0xa9, 0xa6, 0xa5, 0x9a, 0x99, 0x96, 0x95, 0x6a, 0x69, 0x66, 0x65, 0x5a, 0x59, 0x56, 0x60};
					uint8_t fc = FUNCTIONS[function];
					bool ci = function == 1 || function == 3 || function == 4 || function == 5 || function == 7 || function == 13;
					bool explicit_ci = false;
					if ((function == 4 || function == 11) && !BIT(source, 31))
						fc ^= 15;
					if (function == 6 || function == 9)
					{
						if (cl == 0 || cl == 5)
						{
							explicit_ci = true;
						}
						else
						{
							ci = bool(BIT(source, 31)) != (function == 9);
							fc = ci ? 0x99 : 0x66;
						}
					}
					uint32_t const ctl = (uint32_t(modifier) << 28) | (1U << 27) | (uint32_t(fc) << 19) | (uint32_t(ci) << 18) | (uint32_t(explicit_ci) << 14);
					c_input const c_mode = masked ? c_input::MASK : (cl == 0 || cl == 5) ? c_input::EXPAND : c_input::RAW;
					uint32_t const c_value = !masked ? c : (cl == 4) ? d0 : src2;
					auto const r = extended_alu(
							ctl, false, c_mode, (cl == 7) ? 1 : source, (cl == 0 || cl == 2 || cl == 6) ? 0 : shift,
							a, c_value, old_sr, m_regs[0x2a]);
					result = r.value;
					flags = r.flags;
					m_regs[0x2a] = r.mf;
					custom_flags = true;
				}
				else
				{
					if (modifier & 4)
						a = 0;
					if ((modifier & 2) && !shift)
						c = 0xffffffff;
					uint32_t rhs = 0;
					bool sub = false;
					switch (function)
					{
					case 1:
						rhs = b | c;
						sub = true;
						break;
					case 2:
						rhs = b & ~c;
						break;
					case 3:
						rhs = c;
						sub = true;
						break;
					case 4:
						rhs = BIT(source, 31) ? b | ~c : b & c;
						sub = true;
						break;
					case 5:
						rhs = b;
						sub = true;
						break;
					case 6:
						rhs = b;
						sub = cl == 0 || cl == 5 ? BIT(c, 0) : BIT(source, 31);
						break;
					case 7:
						rhs = b & c;
						sub = true;
						break;
					case 8:
						rhs = b & c;
						break;
					case 9:
						rhs = b;
						sub = cl == 0 || cl == 5 ? !BIT(c, 0) : !BIT(source, 31);
						break;
					case 10:
						rhs = b;
						break;
					case 11:
						rhs = BIT(source, 31) ? b | ~c : b & c;
						break;
					case 12:
						rhs = c;
						break;
					case 13:
						rhs = b & ~c;
						sub = true;
						break;
					case 14:
						rhs = b | c;
						break;
					case 15:
						a &= c;
						rhs = b & c;
						break;
					default:
						return unsupported(op, "arithmetic function");
					}
					uint32_t const term = sub ? ~rhs : rhs;
					sum = uint64_t(a) + term + ((modifier & 1) ? BIT(old_sr, 30) : (sub ? 1 : 0));
					result = sum;
					carry = BIT(sum, 32);
					overflow = BIT(~(a ^ term) & (a ^ result), 31);
				}
			}
			if (!custom_flags && (cl == 0 || cl == 5) && BIT(old_sr, 6))
				m_regs[0x2a] = rotate(m_regs[0x2a], count);
		}
		else
		{
			return unsupported(op, "opcode format");
		}
		if (do_alu)
		{
			if (dst != 0x38 && dst != 0x39 && dst != 0x29)
				reg_w(0x29, (old_sr & 0x0fffffff) | (custom_flags ? flags : (result & 0x80000000) | (uint32_t(carry) << 30) | (uint32_t(overflow) << 29) | (uint32_t(!result) << 28)));
			reg_w(dst, result);
		}
		if (mul_dst >= 0)
			reg_w(mul_dst, product);
	}
	for (int i = 0; i < 2; i++)
	{
		auto const &t = d.tr[i];
		if (!t.mode)
			continue;
		if (!t.le)
		{
			if (m_halt_transfer_ready || (m_transfer_pending && i == 1))
				continue; // memory write already completed
			if (!store(t, m_ea[i], transferred[i]))
				return unsupported(op, "transfer size");
		}
		else if (!t.conditional || test)
		{
			reg_w(t.reg, transferred[i]);
			// Only global loads with an unscaled immediate offset can load
			// G/L. Offset bits select which bits to replace (RETI2/3/4).
			if (i == 1 && t.le >= 2 && (t.reg == 0x38 || t.reg == 0x39) && !t.indexed && !t.scale)
			{
				uint32_t const controls = t.index & 3;
				m_fetchpc = (m_fetchpc & ~controls) | (transferred[i] & controls);
			}
		}
	}
	if (d.move_dst >= 0 && (!d.move_conditional || test))
	{
		if (d.move_dst == 15)
		{
			m_poll_safe = false;
			m_master->pp_command(m_pp_number, moved);
		}
		else
		{
			reg_w(d.move_dst, moved);
		}
	}
	return true;
}

void tms320c82_pp_device::start_task()
{
	uint32_t const base = 0x01000000 + (m_pp_number << 12);
	m_program.write_dword(base + 0xffc, m_regs[6]);
	m_program.write_dword(base + 0xff8, m_pipe_pc[1]);
	m_program.write_dword(base + 0xff4, m_pipe_pc[0]);
	m_regs[6] = base + 0xff0;
	m_regs[0x3f] &= ~0x888;
	m_regs[0x3e] = m_pp_number;
	uint32_t const target = m_program.read_dword(base + 0x1b8);
	m_pipe_pc[0] = m_pipe_pc[1] = 0;
	m_pipe_pc[2] = target;
	m_pipe_op[0] = m_pipe_op[1] = NOP;
	m_pipe_op[2] = fetch(target);
	m_fetchpc = target + 8;
	m_ea[0] = m_ea[1] = 0;
	m_irq_next = 0;
	std::fill(std::begin(m_pipe_irq), std::end(m_pipe_irq), 0);
	m_reset_pending = false;
	m_transfer_pending = false;
	m_halt_transfer_ready = false;
}

// HALT lets the first delay-slot memory accesses complete, while its ALU
// and register writes wait for unhalt (SPRU110 3.2.1).
bool tms320c82_pp_device::complete_halt_transfers()
{
	if (m_halt_transfer_ready || m_pipe_irq[0])
		return true;
	auto const &d = decode_cached(m_pipe_pc[0], m_pipe_op[0]);
	bool const test = condition(d.condition);
	// Capture register operands before either port has memory side effects.
	for (int i = 0; i < 2; i++)
	{
		auto const &t = d.tr[i];
		if (!t.mode || (m_transfer_pending && (i == 1 || !t.le)))
			continue;
		uint8_t const source = (!t.le && t.conditional && !test) ? (t.reg & ~1) : t.reg;
		if (!t.le)
			m_transfer_value[i] = reg_r(source);
		else if (t.le == 1)
			m_transfer_value[i] = m_ea[i];
	}
	if (d.move_src >= 0 && !m_transfer_pending)
		m_transfer_value[1] = reg_r(d.move_src);

	auto memory = [this, &d] (int i, bool read, bool write)
	{
		auto const &t = d.tr[i];
		if (!t.mode || (m_transfer_pending && i == 1))
			return true;
		if (read && t.le >= 2)
		{
			if (!load(t, m_ea[i], m_transfer_value[i]))
				return unsupported(m_pipe_op[0], "transfer size");
		}
		else if (write && !t.le)
		{
			if (!store(t, m_ea[i], m_transfer_value[i]))
				return unsupported(m_pipe_op[0], "transfer size");
		}
		return true;
	};
	if (local_diverted(d))
	{
		// The diverted local access follows the global access.
		if (!memory(1, true, true) || !memory(0, true, true))
			return false;
	}
	else
	{
		if (!memory(0, true, false) || !memory(1, true, false)
				|| !memory(0, false, true) || !memory(1, false, true))
			return false;
	}
	m_halt_transfer_ready = true;
	return true;
}

void tms320c82_pp_device::command(uint32_t data)
{
	if (!BIT(data, m_pp_number))
		return;
	if (BIT(data, 31))
	{
		m_halted = true;
		m_reset_pending = true;
		m_transfer_pending = false;
		m_halt_transfer_ready = false;
		m_regs[0x3f] &= ~0x888U;
		m_regs[0x3e] = m_pp_number;
		m_master->packet_reset(m_pp_number);
	}
	else if (BIT(data, 29))
	{
		if (m_reset_pending)
			start_task();
		m_halted = false;
	}
	else if (BIT(data, 30))
	{
		m_halted = true;
	}
	if (BIT(data, 14))
		m_regs[0x3d] |= 1U << 14;
	if (BIT(data, 13))
		m_regs[0x3d] |= 1U << 20;
	if (m_halted && !m_reset_pending && !executing())
		complete_halt_transfers();
}

void tms320c82_pp_device::execute_run()
{
	// Idle loops that only read RAM/ROM repeat exactly while the complete
	// processor state is unchanged, so whole periods can be skipped up to the
	// end of the timeslice.
	uint32_t poll_regs[128], poll_pc[3], poll_ea[2], poll_fetch = 0, poll_protect = 0;
	uint64_t poll_op[3], poll_start = 0;
	bool candidate = false;
	// Very short timeslices aren't worth analysing.
	bool const allow_poll = m_icount > 8;
	m_poll_safe = false;
	while (m_icount > 0)
	{
		if (m_halted)
		{
			m_icount = 0;
			return;
		}
		m_pc = m_pipe_pc[0] & ~7U;
		// An instruction with a diverted local access takes two cycles; stop
		// before the first.
		if (!m_pipe_irq[0] && !m_transfer_pending)
			debugger_instruction_hook(m_pc);
		if (allow_poll && !(machine().debug_flags & DEBUG_FLAG_ENABLED))
		{
			if (m_irq_next || m_pipe_irq[0] || m_pipe_irq[1] || m_pipe_irq[2])
			{
				candidate = false;
			}
			else if (!m_poll_safe)
			{
				// Wait for one eligible instruction before copying all state.
				candidate = false;
				m_poll_safe = true;
			}
			else if (!candidate || m_executed - poll_start > 64)
			{
				std::copy(std::begin(m_regs), std::end(m_regs), poll_regs);
				std::copy(std::begin(m_pipe_pc), std::end(m_pipe_pc), poll_pc);
				std::copy(std::begin(m_pipe_op), std::end(m_pipe_op), poll_op);
				std::copy(std::begin(m_ea), std::end(m_ea), poll_ea);
				poll_fetch = m_fetchpc;
				poll_protect = m_protect;
				poll_start = m_executed;
				candidate = m_poll_safe = true;
			}
			else if (m_fetchpc == poll_fetch && m_protect == poll_protect
					&& std::equal(std::begin(m_pipe_pc), std::end(m_pipe_pc), poll_pc)
					&& std::equal(std::begin(m_regs), std::end(m_regs), poll_regs)
					&& std::equal(std::begin(m_pipe_op), std::end(m_pipe_op), poll_op)
					&& std::equal(std::begin(m_ea), std::end(m_ea), poll_ea))
			{
				uint32_t const period = m_executed - poll_start;
				uint32_t const skipped = (m_icount / period) * period;
				m_executed += skipped;
				m_icount -= skipped;
				if (!m_icount)
					return;
				candidate = false;
			}
		}
		// The first cycle of a diverted local access does the global access.
		// Freeze all pipeline stages; latch global data/store sources only once.
		if (!m_pipe_irq[0] && m_pipe_op[0] != NOP && !m_transfer_pending && !m_halt_transfer_ready)
		{
			auto const &current = decode_cached(m_pipe_pc[0], m_pipe_op[0]);
			if (local_diverted(current))
			{
				auto const &local = current.tr[0], &global = current.tr[1];
				bool const test = condition(current.condition);
				m_transfer_value[0] = reg_r(local.reg);
				uint32_t &value = m_transfer_value[1];
				bool supported = true;
				if (current.move_src >= 0)
				{
					value = reg_r(current.move_src);
				}
				else if (global.le == 1)
				{
					value = m_ea[1];
				}
				else if (!global.le)
				{
					value = reg_r((global.conditional && !test) ? (global.reg & ~1) : global.reg);
					supported = store(global, m_ea[1], value);
				}
				else
				{
					supported = load(global, m_ea[1], value);
				}
				if (!supported)
				{
					unsupported(m_pipe_op[0], "transfer size");
					return;
				}
				m_transfer_pending = true;
				candidate = m_poll_safe = false;
				m_executed++;
				m_icount--;
				continue;
			}
		}
		// EINT/DINT take effect in the fetch stage, before the following
		// instruction is fetched. A return load in E has priority over this.
		uint64_t const fetched_op = m_pipe_op[2];
		if (!m_pipe_irq[2] && (fetched_op & 0xfaa8100000000000ULL) == 0x8800000000000000ULL)
		{
			uint8_t const special = (fetched_op >> 39) & 31;
			if (special == 2)
				m_fetchpc |= 2;
			else if (special == 3)
				m_fetchpc &= ~2U;
		}
		// Address stage runs in parallel with the preceding instruction's
		// execute stage. Explicit register writes have one address-use delay.
		uint32_t next_ea[2]{}, update[2]{};
		int8_t ar[2] = {-1, -1};
		// The canonical NOP has no address-stage or execute-stage work.
		if (m_pipe_op[1] != NOP)
		{
			decoded const &ahead = decode_cached(m_pipe_pc[1], m_pipe_op[1]);
			for (int i = 0; i < 2; i++)
			{
				auto const &t = ahead.tr[i];
				if (!t.mode)
					continue;
				uint32_t const base = t.relative & 2 ? (m_pp_number << 12) | (t.relative == 3 ? 0x01000000 : 0) : reg_r(t.ar);
				uint32_t index = t.indexed ? reg_r(0x10 | (t.ar & 8) | (t.index & 7)) : t.index;
				if (t.scale && t.size)
					index <<= t.size;
				uint32_t const value = (t.mode & 1) ? base - index : base + index;
				next_ea[i] = t.mode < 8 ? base : value;
				if (t.mode < 8 || t.mode >= 12)
				{
					ar[i] = t.ar;
					update[i] = value;
				}
			}
		}
		if (m_pipe_irq[0])
		{
			// The five interrupt pseudoinstructions drain the old pipeline,
			// stack its three continuation addresses and SR, and branch in
			// PS3. The ISR follows PS5 (SPRU110 9.5).
			uint8_t const ps = m_pipe_irq[0];
			if (ps == 1)
				m_irq_sr = m_regs[0x29];
			if (ps <= 4)
			{
				m_regs[6] -= 4;
				uint32_t value = m_irq_sr;
				if (ps == 1)
					value = m_irq_pc[2] & ~1U;
				else if (ps == 2)
					value = m_irq_pc[1] | 1U;
				else if (ps == 3)
					value = m_irq_pc[0] | 1U;
				m_program.write_dword(m_regs[6], value);
			}
			if (ps == 3)
				m_fetchpc = m_program.read_dword(m_irq_vector);
			if (ps == 5)
				m_irq_next = 0;
		}
		else if (m_pipe_op[0] == NOP)
		{
			m_protect = 0;
		}
		else
		{
			if (!execute_one(m_pipe_op[0], decode_cached(m_pipe_pc[0], m_pipe_op[0])))
				return;
		}
		m_transfer_pending = false;
		m_halt_transfer_ready = false;
		for (int i = 0; i < 2; i++)
		{
			if (ar[i] >= 0)
				reg_w(ar[i], update[i]);
			m_ea[i] = next_ea[i];
		}
		uint32_t const pending = m_regs[0x3c] & m_regs[0x3d] & ~1U;
		if (!m_irq_next && BIT(m_fetchpc, 1) && pending)
		{
			m_irq_vector = 0x01000180 + (m_pp_number << 12) + std::countr_zero(pending) * 4;
			m_irq_next = 1;
		}
		m_pipe_pc[0] = m_pipe_pc[1];
		m_pipe_pc[1] = m_pipe_pc[2];
		m_pipe_pc[2] = m_fetchpc;
		m_pipe_op[0] = m_pipe_op[1];
		m_pipe_op[1] = m_pipe_op[2];
		m_pipe_irq[0] = m_pipe_irq[1];
		m_pipe_irq[1] = m_pipe_irq[2];
		uint8_t const ps = m_irq_next && m_irq_next <= 5 ? m_irq_next++ : 0;
		m_pipe_irq[2] = ps;
		m_pipe_op[2] = ps ? NOP : fetch(m_fetchpc);
		uint32_t const fetched = m_fetchpc;
		if (m_poll_safe && (!m_cache.read_ptr(fetched & ~7U) || !m_cache.read_ptr((fetched & ~7U) + 4)))
			m_poll_safe = false;
		if (ps && ps <= 3)
			m_irq_pc[ps - 1] = fetched;
		if (ps <= 3)
			m_fetchpc += 8;
		// RETI4 fetches the final saved PC with L=0, so its loop
		// decision must be deferred until then. IPE/IPA replay with L=1.
		if (ps < 3 && !BIT(fetched, 0))
		{
			// Coincident loop ends have priority from le2 down to le0. Matching
			// loops with a zero count are reloaded until one with a nonzero
			// count (or no counter) branches (SPRU110 5.3.2, 11.4.1).
			for (int n = 2; n >= 0; n--)
			{
				if (!BIT(m_regs[0x3f], n * 4 + 3) || (fetched & ~7U) != (m_regs[0x74 + n] & ~7U))
					continue;
				uint8_t const counter = BIT(m_regs[0x3f], n * 4, 3);
				if (!counter || (counter <= 3 && m_regs[0x60 + counter - 1]))
				{
					m_fetchpc = (m_regs[0x70 + n] & ~7U) | (fetched & 7);
					if (counter)
						--m_regs[0x60 + counter - 1];
					break;
				}
				if (counter <= 3)
					m_regs[0x60 + counter - 1] = m_regs[0x64 + counter - 1];
			}
		}
		m_executed++;
		m_icount--;
		if (m_halted && !m_reset_pending && !complete_halt_transfers())
			return;
	}
}
