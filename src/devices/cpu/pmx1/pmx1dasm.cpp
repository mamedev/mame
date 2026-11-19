// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    VideoLogic PMX1 primary core disassembler

    VideoLogic did not publish the instruction set of the primary core, and
    no assembler syntax for it is known.  The encoding is known as far as
    it has been worked out from the microcode of the Neon 250 drivers and the
    Gaelco games; the syntax here is made up.

    Registers
      da0-da31, db0-db31    the two banks of data registers (units 1 and 2)
      aa0-aa15, ab0-ab15    the two banks of address registers (units 3 and 4)
      pc0-pc3               the program counter of each thread (unit 5)
      rq                    the executing thread's queue of memory reads (unit 6):
                            written with an address to queue a read, read to take the oldest
      ans                   what the last read of an implicit register gave (unit 8)
      kick0-kick3           taking a kick from a thread's kicker, or waiting for one (unit 10)
      kick                  the executing thread's own kicker, left as it is
      cycles                the cycle counter (unit 11)

    Instructions
      add sub and or xor shl shr    destination, source, source; or destination, 16-bit immediate
      mov, movn                     load a register with a register, an immediate or minus an immediate
      cmp, tst                      subtract or AND for the flags only
      b                             jump relative to the instruction
      mov, xchg, call               between units: move, exchange, exchange with the program counter
      mov64                         64 bits from the read queue or the tile accelerator's status
      imp                           a register holds a read or write command for an implicit register
      ld, st, st64                  memory at [pointer + offset]; [pointer += offset] also moves the pointer
      fill                          repeated store, moving the pointer before each
      tacmd                         a 64-bit instruction for the tile accelerator
      out                           write a port register
      feed                          give the oldest read to the display feed and queue another;
                                    [pointer += offset] repeats, moving the pointer before each
      nop, frag                     nothing; frag is the word that starts each fragment of code

    A mnemonic ending in .f sets the flags; .eq .ne .cs .cc .mi .pl .vs .vc
    .hi .ls .ge .lt .gt .le make the instruction conditional.  Thread 0 counts the address a jump goes to from
    four instructions past the jump: without a processor to ask, addresses
    are shown as threads 1 to 3 would take them.
*/

#include "emu.h"
#include "pmx1dasm.h"


namespace {

constexpr char const *const OPERATIONS[7] = { "add", "sub", "and", "or", "xor", "shl", "shr" };

constexpr char const *const BANKS[5] = { nullptr, "da", "db", "aa", "ab" };

// condition codes, those of the ARM plus one: only those with a name are known
constexpr char const *const CONDITIONS[16] = {
		"", "eq", "ne", "cs", "cc", "mi", "pl", "vs",
		"vc", "hi", "ls", "ge", "lt", "gt", "le", nullptr };

} // anonymous namespace


pmx1core_disassembler::pmx1core_disassembler(const config *conf) :
	m_config(conf)
{
}

u32 pmx1core_disassembler::opcode_alignment() const
{
	return 4;
}

u32 pmx1core_disassembler::operand16(u32 op)
{
	// bit 1 extends the sign of the 16 bits, bit 0 then moves them to the upper half
	const u32 value = BIT(op, 1) ? u32(util::sext(BIT(op, 3, 16), 16)) : BIT(op, 3, 16);
	return BIT(op, 0) ? (value << 16) : value;
}

std::string pmx1core_disassembler::reg(u32 unit, u32 number)
{
	// empty when there is no such register
	if ((unit < 1) || (unit > 4) || ((unit > 2) && (number > 15)))
		return std::string();
	return util::string_format("%s%u", BANKS[unit], number);
}

std::string pmx1core_disassembler::target(u32 unit, u32 number)
{
	// where the result of an instruction can go besides the registers
	if ((unit == 5) && (number < 4))
		return util::string_format("pc%u", number);
	else if ((unit == 6) && !number)
		return "rq";
	else
		return reg(unit, number);
}

std::string pmx1core_disassembler::pointer(u32 selector)
{
	// five bits select one of all the address registers
	return reg(3 + BIT(selector, 4), selector & 15);
}

std::string pmx1core_disassembler::mnemonic(const char *name, u32 condition)
{
	// empty when the condition code is not known
	if (!CONDITIONS[condition])
		return std::string();
	return condition ? util::string_format("%s.%s", name, CONDITIONS[condition]) : std::string(name);
}

bool pmx1core_disassembler::arithmetic(std::ostream &stream, u32 op)
{
	// majors 0 to 6 are the operations on data registers, major 9 adds address registers
	const u32 major = BIT(op, 28, 4);
	const u32 operation = (major == 9) ? 0 : major;
	const u32 unit = ((major == 9) ? 3 : 1) + BIT(op, 24);
	const bool flags = BIT(op, 27), conditional = BIT(op, 26), immediate = BIT(op, 25);
	const bool shift = operation >= 5;
	const bool to_unit = conditional && immediate && !shift && BIT(op, 5);
	if (flags && ((major == 9) || (conditional && !to_unit)))
		return false;

	const std::string name = flags ? util::string_format("%s.f", OPERATIONS[operation]) : std::string(OPERATIONS[operation]);
	const std::string d = reg(unit, BIT(op, 19, 5)), s1 = reg(unit, BIT(op, 14, 5)), s2 = reg(unit, BIT(op, 9, 5));

	if (!conditional && !immediate)
	{
		// two registers, or with bit 2 a copy of the second
		if (op & 0x1fb)
			return false;
		if (BIT(op, 2))
		{
			if (flags || shift || d.empty() || s2.empty())
				return false;
			util::stream_format(stream, "%-8s%s, %s", "mov", d, s2);
		}
		else
		{
			if (d.empty() || s1.empty() || s2.empty())
				return false;
			util::stream_format(stream, "%-8s%s, %s, %s", name, d, s1, s2);
		}
	}
	else if (!conditional)
	{
		if (shift)
		{
			// the count is in the instruction
			if (BIT(op, 0, 9))
				return false;
			util::stream_format(stream, "%-8s%s, %s, %u", name, d, s1, BIT(op, 9, 5));
		}
		else if (BIT(op, 2))
		{
			// load the 16-bit operand or what it leaves when taken from zero
			if ((operation > 1) || flags || d.empty())
				return false;
			util::stream_format(stream, "%-8s%s, 0x%x", operation ? "movn" : "mov", d, operand16(op));
		}
		else
		{
			if (d.empty())
				return false;
			util::stream_format(stream, "%-8s%s, 0x%x", name, d, operand16(op));
		}
	}
	else if (immediate && !shift && BIT(op, 5))
	{
		// 8-bit operand, the result can go to another unit
		const std::string to = target(BIT(op, 1, 4), BIT(op, 19, 5));
		if ((operation > 2) || to.empty() || s1.empty())
			return false;
		util::stream_format(stream, "%-8s%s, %s, 0x%x", name, to, s1, BIT(op, 6, 8));
	}
	else
	{
		// the conditional forms
		const std::string cond_name = mnemonic(OPERATIONS[operation], BIT(op, 1, 4));
		if (cond_name.empty())
			return false;
		if (immediate && shift)
		{
			const std::string to = reg(BIT(op, 5, 4), BIT(op, 19, 5));
			if (to.empty())
				return false;
			util::stream_format(stream, "%-8s%s, %s, %u", cond_name, to, s1, BIT(op, 9, 5));
		}
		else if (immediate)
		{
			// what bit 0 does is not known
			if (d.empty() || s1.empty())
				return false;
			util::stream_format(stream, "%-8s%s, %s, 0x%x", cond_name, d, s1, BIT(op, 6, 8));
		}
		else
		{
			const std::string to = target(BIT(op, 5, 4), BIT(op, 19, 5));
			if (shift || to.empty() || s1.empty() || s2.empty())
				return false;
			util::stream_format(stream, "%-8s%s, %s, %s", cond_name, to, s1, s2);
		}
	}
	return true;
}

bool pmx1core_disassembler::compare(std::ostream &stream, u32 op)
{
	const char *const name = BIT(op, 27) ? "tst" : "cmp";
	const u32 unit = 1 + BIT(op, 24);
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5), s2 = BIT(op, 9, 5);
	switch (BIT(op, 25, 2))
	{
	case 0:
		if (d || BIT(op, 0, 9))
			return false;
		util::stream_format(stream, "%-8s%s, %s", name, reg(unit, s1), reg(unit, s2));
		return true;

	case 1:
		if (BIT(op, 2))
			return false;
		util::stream_format(stream, "%-8s%s, 0x%x", name, reg(unit, d), operand16(op));
		return true;

	case 3:
		{
			const std::string cond_name = mnemonic(name, BIT(op, 1, 4));
			if (d || BIT(op, 5) || BIT(op, 0) || cond_name.empty())
				return false;
			util::stream_format(stream, "%-8s%s, 0x%x", cond_name, reg(unit, s1), BIT(op, 6, 8));
			return true;
		}

	default:
		return false;
	}
}

bool pmx1core_disassembler::move(std::ostream &stream, u32 op, u32 &flags) const
{
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5), s2 = BIT(op, 9, 5);
	const u32 condition = BIT(op, 1, 4);
	switch (BIT(op, 24, 4))
	{
	case 4:
		{
			const u32 from = BIT(op, 5, 4), to = BIT(op, 10, 4);
			const bool exchange = BIT(op, 9), call = exchange && (to == 5);
			const std::string name = mnemonic(call ? "call" : (exchange ? "xchg" : "mov"), condition);
			std::string dest, source;
			if (exchange)
			{
				// two registers change places, or a register and the program counter
				dest = call ? target(to, d) : reg(to, d);
				source = reg(from, s1);
			}
			else
			{
				// unit 0 as the destination is memory
				dest = to ? target(to, d) : util::string_format("[%s]", pointer(d));
				switch (from)
				{
				case 6:
					source = "rq";
					break;
				case 8:
					if (s1 == 2)
						source = "ans";
					break;
				case 10:
					if (s1 < 4)
						source = util::string_format("kick%u", s1);
					else if (s1 == 7)
						source = "kick";
					break;
				case 11:
					if (!s1)
						source = "cycles";
					break;
				default:
					source = reg(from, s1);
					break;
				}
			}
			if (name.empty() || dest.empty() || source.empty())
				return false;
			util::stream_format(stream, "%-8s%s, %s", name, dest, source);

			// a real-time thread comes back to the next instruction
			if (call && (!m_config || !m_config->jumps_delayed()))
				flags |= condition ? (STEP_OVER | STEP_COND) : STEP_OVER;
			return true;
		}

	case 5:
		{
			const std::string name = mnemonic("imp", condition);
			const std::string command = reg(BIT(op, 5, 4), s1);
			if (d || s2 || BIT(op, 0) || name.empty() || command.empty())
				return false;
			util::stream_format(stream, "%-8s%s", name, command);
			return true;
		}

	case 14:
		if ((BIT(op, 5, 4) == 6) && !d)
			util::stream_format(stream, "%-8sda%u, db%u, rq", "mov64", s1, s2);
		else if ((BIT(op, 5, 4) == 8) && (d == 4))
			util::stream_format(stream, "%-8sda%u, db%u, ta", "mov64", s1, s2);
		else
			return false;
		return true;

	default:
		return false;
	}
}

bool pmx1core_disassembler::memory(std::ostream &stream, u32 op)
{
	const bool load = BIT(op, 28, 4) == 12;
	const u32 minor = BIT(op, 24, 4);
	const u32 unit = BIT(op, 1, 4), d = BIT(op, 19, 5);
	const std::string base = reg(3 + BIT(op, 18), BIT(op, 14, 4));
	const std::string index = reg(3 + BIT(op, 18), BIT(op, 9, 4));
	const char *const plus = BIT(op, 26) ? "+=" : "+";

	if (!load && (minor == 12))
	{
		if (BIT(op, 13) || (BIT(op, 0, 9) != 0x059))
			return false;
		util::stream_format(stream, "%-8s[%s += %s], db%u", "fill", base, index, d);
		return true;
	}
	if (!(minor & 0xb))
	{
		// the offset is in a register of the same bank
		const std::string data = reg(unit, d);
		if (BIT(op, 13) || BIT(op, 6, 3) || !BIT(op, 5) || BIT(op, 0) || data.empty())
			return false;
		if (load)
			util::stream_format(stream, "%-8s%s, [%s %s %s]", "ld", data, base, plus, index);
		else
			util::stream_format(stream, "%-8s[%s %s %s], %s", "st", base, plus, index, data);
		return true;
	}

	// the other forms have the offset in the instruction, in words
	if (BIT(op, 6, 2) || !BIT(op, 5) || BIT(op, 0))
		return false;
	const std::string address = util::string_format("[%s %s 0x%x]", base, plus, 4 * BIT(op, 8, 6));
	const std::string data = (unit == 6) ? std::string("rq") : reg(unit, d);
	if (load && ((minor & 0xb) == 2) && (unit != 6) && !data.empty())
		util::stream_format(stream, "%-8s%s, %s", "ld", data, address);
	else if (!load && ((minor & 0xb) == 2) && !data.empty())
		util::stream_format(stream, "%-8s%s, %s", "st", address, data);
	else if (!load && (minor == 7) && (unit == 6))
		util::stream_format(stream, "%-8s%s, %s", "st64", address, data);
	else
		return false;
	return true;
}

bool pmx1core_disassembler::port(std::ostream &stream, u32 op)
{
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5);
	switch (BIT(op, 24, 4))
	{
	case 2:
		if (BIT(op, 0, 14) == 0x0004)
			util::stream_format(stream, "%-8saa%u, ab%u", "tacmd", d & 15, s1 & 15);
		else if (!BIT(op, 0, 3) && BIT(op, 5) && BIT(op, 4) && ((BIT(op, 6, 8) == 0x64) || (BIT(op, 6, 8) == 0x65)))
			util::stream_format(stream, "%-8s0x%02x, %s", "out", BIT(op, 6, 8), BIT(op, 3) ? reg(1, d) : reg(3, d & 15));
		else
			return false;
		return true;

	case 4:
		if (BIT(op, 0, 14) != 0x000b)
			return false;
		util::stream_format(stream, "%-8s[%s]", "feed", pointer(d));
		return true;

	case 13:
		if (BIT(op, 0, 14) != 0x1c03)
			return false;
		util::stream_format(stream, "%-8s[%s += %s]", "feed", pointer(d), reg(3 + BIT(d, 4), s1 & 15));
		return true;

	default:
		return false;
	}
}

bool pmx1core_disassembler::branch(std::ostream &stream, offs_t pc, u32 op, u32 &flags) const
{
	if (op == 0xf100'0000)
	{
		stream << "nop";
		return true;
	}
	if ((op & 0xff07'ffff) == 0xf100'0002)
	{
		util::stream_format(stream, "%-8s%u", "frag", BIT(op, 19, 5));
		return true;
	}

	const u32 condition = BIT(op, 1, 4);
	const std::string name = mnemonic("b", condition);
	if (BIT(op, 24, 4) || BIT(op, 19, 5) || BIT(op, 0) || name.empty())
		return false;
	const u32 from = pc + ((m_config && m_config->jumps_delayed()) ? 16 : 0);
	util::stream_format(stream, "%-8s0x%08x", name, from + (u32(util::sext(BIT(op, 5, 14), 14)) << 2));
	if (condition)
		flags |= STEP_COND;
	return true;
}

offs_t pmx1core_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u32 op = opcodes.r32(pc);
	u32 flags = 0;
	bool known;
	switch (BIT(op, 28, 4))
	{
	case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 9:
		known = arithmetic(stream, op);
		break;

	case 8:
		known = compare(stream, op);
		break;

	case 10:
		known = move(stream, op, flags);
		break;

	case 11: case 12:
		known = memory(stream, op);
		break;

	case 14:
		known = port(stream, op);
		break;

	case 15:
		known = branch(stream, pc, op, flags);
		break;

	default:
		known = false;
		break;
	}
	if (!known)
		util::stream_format(stream, "%-8s0x%08x", ".word", op);

	return 4 | flags | SUPPORTED;
}
