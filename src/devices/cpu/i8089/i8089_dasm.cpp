// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 8089 I/O Processor

    Disassembler

***************************************************************************/

#include "emu.h"
#include "i8089_dasm.h"

const char *const i8089_disassembler::m_reg[] =
{
	"ga", "gb", "gc", "bc", "tp", "ix", "cc", "mc"
};

// fetch 1-byte value
uint8_t i8089_disassembler::fetch_value8()
{
	uint8_t i = m_opcodes->r8(m_pc);
	m_pc += 1;
	return i;
}

// fetch 2-byte value
uint16_t i8089_disassembler::fetch_value16()
{
	uint16_t i = m_opcodes->r16(m_pc);
	m_pc += 2;
	return i;
}

// fetch a 1 or 2 byte immediate value
uint16_t i8089_disassembler::fetch_immediate()
{
	return (m_wb & 1) ? fetch_value8() : fetch_value16();
}

// print memory offset
std::string i8089_disassembler::offset()
{
	const char *mm_name[]  = { "ga", "gb", "gc", "pp" };

	switch (m_aa)
	{
	case 0: return util::string_format("[%s]", mm_name[m_mm]);
	case 1: return util::string_format("[%s].%02x", mm_name[m_mm], m_opcodes->r8(m_pc++));
	case 2: return util::string_format("[%s+ix]", mm_name[m_mm]);
	case 3: return util::string_format("[%s+ix+]", mm_name[m_mm]);
	}
	return "";
}

// invalid instruction
std::string i8089_disassembler::invalid()
{
	return "???";
}

// to register or memory from immediate
std::string i8089_disassembler::from_i(const std::string &instr8, const std::string &instr16, const std::string &target)
{
	if (m_w == 0 && m_wb == 1)
		return util::string_format("%s %s, %02x", instr8, target, fetch_immediate());
	else if (m_w == 1 && m_wb == 2)
		return util::string_format("%s %s, %04x", instr16, target, fetch_immediate());
	else
		return invalid();
}

// register, immediate
std::string i8089_disassembler::inst_ri(const std::string &instr8, const std::string &instr16)
{
	if (m_mm == 0)
		return from_i(instr8, instr16, m_reg[m_brp]);
	else
		return invalid();
}

// register
std::string i8089_disassembler::inst_r(const std::string &instr)
{
	if (m_mm == 0 && m_w == 0 && m_aa == 0 && m_wb == 0)
		return util::string_format("%s %s", instr, m_reg[m_brp]);
	else
		return invalid();
}

// jump register
std::string i8089_disassembler::inst_jr(const std::string &instr8, const std::string &instr16)
{
	uint16_t i = fetch_immediate();
	if (m_mm == 0 && m_w == 0 && m_wb == 1)
		return util::string_format("%s %s, %05x", instr8, m_reg[m_brp], m_pc + (int8_t) i);
	else if (m_mm == 0 && m_w == 0 && m_wb == 2)
		return util::string_format("%s %s, %05x", instr16, m_reg[m_brp], m_pc + (int16_t) i);
	else
		return invalid();
}

// memory immediate
std::string i8089_disassembler::inst_mi(const std::string &instr8, const std::string &instr16)
{
	std::string off = offset();
	return from_i(instr8, instr16, off);
}

// register memory
std::string i8089_disassembler::inst_rm(const std::string &instr8, const std::string &instr16)
{
	std::string off = offset();
	if (m_w == 0 && m_wb == 0)
		return util::string_format("%s %s, %s", instr8, m_reg[m_brp], off);
	else if (m_w == 1 && m_wb == 0)
		return util::string_format("%s %s, %s", instr16, m_reg[m_brp], off);
	else
		return invalid();
}

// jump memory
std::string i8089_disassembler::inst_jm(const std::string &jump8short, const std::string &jump8long)
{
	std::string off = offset();
	uint16_t i = fetch_immediate();
	if (m_w == 0 && m_wb == 1 && m_brp == 0)
		return util::string_format("%s %s, %05x", jump8short, off, m_pc + (int8_t) i);
	else if (m_w == 0 && m_wb == 2 && m_brp == 0)
		return util::string_format("%s %s, %05x", jump8long, off, m_pc + (int16_t) i);
	else
		return invalid();
}

// jump memory bit
std::string i8089_disassembler::inst_jmb(const std::string &jump8short, const std::string &jump8long)
{
	std::string off = offset();
	uint16_t i = fetch_immediate();
	if (m_w == 0 && m_wb == 1)
		return util::string_format("%s %s, %d, %05x", jump8short, off, m_brp, m_pc + (int8_t) i);
	else if (m_w == 0 && m_wb == 2)
		return util::string_format("%s %s, %d, %05x", jump8long, off, m_brp, m_pc + (int16_t) i);
	else
		return invalid();
}

// memory register
std::string i8089_disassembler::inst_mr(const std::string &instr8, const std::string &instr16)
{
	std::string off = offset();
	if (m_w == 0 && m_wb == 0)
		return util::string_format("%s %s, %s", instr8, off, m_reg[m_brp]);
	else if (m_w == 1 && m_wb == 0)
		return util::string_format("%s %s, %s", instr16, off, m_reg[m_brp]);
	else
		return invalid();
}

// pointer memory
std::string i8089_disassembler::inst_pm(const std::string &instr16)
{
	std::string off = offset();
	if (m_w == 1 && m_wb == 0)
		return util::string_format("%s %s, %s", instr16, m_reg[m_brp], off);
	else
		return invalid();
}

// memory pointer
std::string i8089_disassembler::inst_mp(const std::string &instr16)
{
	std::string off = offset();
	if (m_w == 1 && m_wb == 0)
		return util::string_format("%s %s, %s", instr16, off, m_reg[m_brp]);
	else
		return invalid();
}

// jump memory
std::string i8089_disassembler::inst_j16(const std::string &jump8short, const std::string &jump16short, const std::string &jump8long, const std::string &jump16long)
{
	std::string off = offset();
	uint16_t i = fetch_immediate();
	if (m_w == 0 && m_wb == 1)
		return util::string_format("%s %s, %05x", jump8short, off, m_pc + (int8_t) i);
	else if (m_w == 1 && m_wb == 1)
		return util::string_format("%s %s, %05x", jump16short, off, m_pc + (int8_t) i);
	else if (m_w == 0 && m_wb == 2)
		return util::string_format("%s %s, %05x", jump8long, off, m_pc + (int16_t) i);
	else if (m_w == 1 && m_wb == 2)
		return util::string_format("%s %s, %05x", jump16long, off, m_pc + (int16_t) i);
	else
		return invalid();
}

// memory
std::string i8089_disassembler::inst_m(const std::string &instr8, const std::string &instr16)
{
	std::string off = offset();
	if (m_w == 0 && m_wb == 0)
		return util::string_format("%s %s", instr8, off);
	else if (m_w == 1 && m_wb == 0)
		return util::string_format("%s %s", instr16, off);
	else
		return invalid();
}

// memory bit
std::string i8089_disassembler::inst_b(const std::string &instr)
{
	std::string off = offset();
	if (m_w == 0 && m_wb == 0)
		return util::string_format("%s %s, %d", instr, off, m_brp);
	else
		return invalid();
}

u32 i8089_disassembler::opcode_alignment() const
{
	return 1;
}

void i8089_disassembler::load_instruction()
{
	// instruction
	m_brp = (m_opcodes->r8(m_pc)   >> 5) & 0x07;
	m_wb  = (m_opcodes->r8(m_pc)   >> 3) & 0x03;
	m_aa  = (m_opcodes->r8(m_pc)   >> 1) & 0x03;
	m_w   = (m_opcodes->r8(m_pc)   >> 0) & 0x01;
	m_opc = (m_opcodes->r8(m_pc+1) >> 2) & 0x3f;
	m_mm  = (m_opcodes->r8(m_pc+1) >> 0) & 0x03;
	m_pc += 2;
}

offs_t i8089_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	m_opcodes = &opcodes;
	m_pc = pc;
	m_flags = SUPPORTED;

	load_instruction();
	stream << do_disassemble();
	return (m_pc - pc) | m_flags;
}

std::string i8089_disassembler::do_disassemble()
{
	switch (m_opc)
	{
	case 0x00:
		if (m_mm == 0 && m_w == 0 && m_aa == 0 && m_wb == 0)
		{
			switch (m_brp)
			{
			case 0: return util::string_format("nop");
			case 1: return invalid(); break;
			case 2: return util::string_format("sintr");
			case 3: return util::string_format("xfer");
			case 4: return util::string_format("wid 8, 8");
			case 5: return util::string_format("wid 8, 16");
			case 6: return util::string_format("wid 16, 8");
			case 7: return util::string_format("wid 16, 16");
			}
		}
		else
			return invalid();
		break;

	case 0x02:
		if (m_w == 1 && m_aa == 0 && m_wb == 2)
		{
			uint16_t offs = fetch_immediate();
			uint16_t segm = fetch_immediate();
			return util::string_format("lpdi %s, %05x", m_reg[m_brp], ((segm << 4) + offs) & 0xfffff);
		}
		else
			return invalid();

	case 0x08:
		if (m_mm == 0)
		{
			uint16_t i = fetch_immediate();

			if (m_w == 0 && m_aa == 0 && m_wb == 1 && m_brp == TP)
				return util::string_format("jmp %05x", m_pc + (int8_t) i);
			else if (m_w == 1 && m_aa == 0 && m_wb == 2 && m_brp == TP)
				return util::string_format("ljmp %05x", m_pc + (int16_t) i);
			else if (m_w == 0 && m_wb == 1)
				return util::string_format("addbi %s, %02x", m_reg[m_brp], i);
			else if (m_w == 1 && m_wb == 2)
				return util::string_format("addi %s, %04x", m_reg[m_brp], i);
			else
				return invalid();
			}
		else
			return invalid();

	case 0x09: return inst_ri("orbi", "ori");
	case 0x0a: return inst_ri("andbi", "andi");
	case 0x0b: return inst_r("not");
	case 0x0c: return inst_ri("movbi", "movi");
	case 0x0e: return inst_r("inc");
	case 0x0f: return inst_r("dec");
	case 0x10: m_flags |= STEP_COND; return inst_jr("jnz", "ljnz");
	case 0x11: m_flags |= STEP_COND; return inst_jr("jz", "ljz");

	case 0x12:
		if (m_mm == 0 && m_w == 0 && m_aa == 0 && m_wb == 0 && m_brp == 1)
			return util::string_format("hlt");
		else
			return invalid();

	case 0x13: return inst_mi("movbi", "movi");
	case 0x20: return inst_rm("movb", "mov");
	case 0x21: return inst_mr("movb", "mov");
	case 0x22: return inst_pm("lpd");
	case 0x23: return inst_pm("movp");

	case 0x24:
		if (m_wb == 0)
		{
			std::string off = offset();

			load_instruction();

			if (m_opc != 0x33)
				return invalid();
			std::string off2 = offset();

			if (m_w == 0)
				return util::string_format("movb %s, %s", off2, off);
			else
				return util::string_format("mov %s, %s", off2, off);
		}
		else
			return invalid();

	case 0x25:
		if (m_w == 0 && m_wb == 3 && m_brp == 0)
		{
			std::string off = offset();
			uint16_t i = fetch_immediate();
			int displacement = (int8_t) fetch_immediate();
			return util::string_format("tsl %s, %02x, %05x", off, i, m_pc + displacement);
		}
		else
			return invalid();

	case 0x26: return inst_mp("movp");

	case 0x27:
		if (m_w == 1 && m_brp == TP && (m_wb == 1 || m_wb == 2))
		{
			std::string off = offset();
			uint16_t i = fetch_immediate();

			m_flags |= STEP_OVER;

			if (m_wb == 1)
				return util::string_format("call %s, %05x", off, m_pc + (int8_t) i);
			else if (m_wb == 2)
				return util::string_format("lcall %s, %05x", off, m_pc + (int16_t) i);
		}
		return invalid();

	case 0x28: return inst_rm("addb", "add");
	case 0x29: return inst_rm("orb", "or");
	case 0x2a: return inst_rm("andb", "and");
	case 0x2b: return inst_rm("notb", "not");
	case 0x2c: m_flags |= STEP_COND; return inst_jm("jmce", "ljmce");
	case 0x2d: m_flags |= STEP_COND; return inst_jm("jmcne", "ljmcne");
	case 0x2e: m_flags |= STEP_COND; return inst_jmb("jnbt", "ljnbt");
	case 0x2f: m_flags |= STEP_COND; return inst_jmb("jbt", "ljbt");
	case 0x30: return inst_mi("addbi", "addi");
	case 0x31: return inst_mi("orbi", "ori");
	case 0x32: return inst_mi("andbi", "andi");

	case 0x34: return inst_mr("addb", "add");
	case 0x35: return inst_mr("orb", "or");
	case 0x36: return inst_mr("andb", "and");
	case 0x37: return inst_mr("notb", "not");
	case 0x38: m_flags |= STEP_COND; return inst_j16("jnzb", "jnz", "ljnzb", "ljnz");
	case 0x39: m_flags |= STEP_COND; return inst_j16("jzb", "jz", "ljzb", "ljz");
	case 0x3a: return inst_m("incb", "inc");
	case 0x3b: return inst_m("decb", "dec");
	case 0x3d: return inst_b("setb");
	case 0x3e: return inst_b("clr");
	}

	return invalid();
}
