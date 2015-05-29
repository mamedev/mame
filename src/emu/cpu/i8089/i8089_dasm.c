// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 8089 I/O Processor

    Disassembler

***************************************************************************/

#include "emu.h"

class i8089_instruction
{
public:
	i8089_instruction(offs_t pc, const UINT8 *oprom) :
		m_oprom(oprom), m_ppc(pc), m_pc(0), m_flags(DASMFLAG_SUPPORTED)
	{
		// instruction
		m_brp = (oprom[0] >> 5) & 0x07;
		m_wb  = (oprom[0] >> 3) & 0x03;
		m_aa  = (oprom[0] >> 1) & 0x03;
		m_w   = (oprom[0] >> 0) & 0x01;
		m_opc = (oprom[1] >> 2) & 0x3f;
		m_mm  = (oprom[1] >> 0) & 0x03;

		// clear buffers
		memset(m_buffer, 0, sizeof(m_buffer));
		memset(m_offset, 0, sizeof(m_offset));

		// start working
		disassemble();
	}

	char *buffer() { return m_buffer; }
	int length() { return m_pc; }
	int flags() { return m_flags; }

private:
	const UINT8 *m_oprom;

	char m_buffer[256];
	char m_offset[100];
	offs_t m_ppc;
	int m_pc;

	// decoded instruction
	int m_brp;
	int m_wb;
	int m_aa;
	int m_w;
	int m_opc;
	int m_mm;

	// dasm flags
	int m_flags;

	// register names
	static const char *m_reg[];

	// register index
	enum
	{
		GA,  // 20-bit general purpose address a
		GB,  // 20-bit general purpose address b
		GC,  // 20-bit general purpose address c
		BC,  // byte count
		TP,  // 20-bit task pointer
		IX,  // index
		CC,  // mask compare
		MC   // channel control
	};

	// fetch 1-byte value
	UINT8 fetch_value8()
	{
		UINT8 i = m_oprom[m_pc];
		m_pc += 1;
		return i;
	}

	// fetch 2-byte value
	UINT16 fetch_value16()
	{
		UINT16 i = m_oprom[m_pc] | m_oprom[m_pc + 1] << 8;
		m_pc += 2;
		return i;
	}

	// fetch a 1 or 2 byte immediate value
	UINT16 fetch_immediate()
	{
		return (m_wb & 1) ? fetch_value8() : fetch_value16();
	}

	// print memory offset
	void offset()
	{
		const char *mm_name[]  = { "ga", "gb", "gc", "pp" };

		switch (m_aa)
		{
		case 0: sprintf(m_offset, "[%s]", mm_name[m_mm]); break;
		case 1: sprintf(m_offset, "[%s].%02x", mm_name[m_mm], m_oprom[m_pc]); m_pc++; break;
		case 2: sprintf(m_offset, "[%s+ix]", mm_name[m_mm]); break;
		case 3: sprintf(m_offset, "[%s+ix+]", mm_name[m_mm]); break;
		}
	}

	// to register or memory from immediate
	void from_i(const char *instr8, const char *instr16, const char *target)
	{
		if (m_w == 0 && m_wb == 1)
			sprintf(m_buffer, "%s %s, %02x", instr8, target, fetch_immediate());
		else if (m_w == 1 && m_wb == 2)
			sprintf(m_buffer, "%s %s, %04x", instr16, target, fetch_immediate());
		else
			invalid();
	}

	// register, immediate
	void inst_ri(const char *instr8, const char *instr16)
	{
		if (m_mm == 0)
			from_i(instr8, instr16, m_reg[m_brp]);
		else
			invalid();
	}

	// register
	void inst_r(const char *instr)
	{
		if (m_mm == 0 && m_w == 0 && m_aa == 0 && m_wb == 0)
			sprintf(m_buffer, "%s %s", instr, m_reg[m_brp]);
		else
			invalid();
	}

	// jump register
	void inst_jr(const char *instr8, const char *instr16)
	{
		UINT16 i = fetch_immediate();
		if (m_mm == 0 && m_w == 0 && m_wb == 1)
			sprintf(m_buffer, "%s %s, %05x", instr8, m_reg[m_brp], m_ppc + m_pc + (INT8) i);
		else if (m_mm == 0 && m_w == 0 && m_wb == 2)
			sprintf(m_buffer, "%s %s, %05x", instr16, m_reg[m_brp], m_ppc + m_pc + (INT16) i);
		else
			invalid();
	}

	// memory immediate
	void inst_mi(const char *instr8, const char *instr16)
	{
		offset();
		from_i(instr8, instr16, m_offset);
	}

	// register memory
	void inst_rm(const char *instr8, const char *instr16)
	{
		offset();
		if (m_w == 0 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %s", instr8, m_reg[m_brp], m_offset);
		else if (m_w == 1 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %s", instr16, m_reg[m_brp], m_offset);
		else
			invalid();
	}

	// jump memory
	void inst_jm(const char *jump8short, const char *jump8long)
	{
		offset();
		UINT16 i = fetch_immediate();
		if (m_w == 0 && m_wb == 1 && m_brp == 0)
			sprintf(m_buffer, "%s %s, %05x", jump8short, m_offset, m_ppc + m_pc + (INT8) i);
		else if (m_w == 0 && m_wb == 2 && m_brp == 0)
			sprintf(m_buffer, "%s %s, %05x", jump8long, m_offset, m_ppc + m_pc + (INT16) i);
		else
			invalid();
	}

	// jump memory bit
	void inst_jmb(const char *jump8short, const char *jump8long)
	{
		offset();
		UINT16 i = fetch_immediate();
		if (m_w == 0 && m_wb == 1)
			sprintf(m_buffer, "%s %s, %d, %05x", jump8short, m_offset, m_brp, m_ppc + m_pc + (INT8) i);
		else if (m_w == 0 && m_wb == 2)
			sprintf(m_buffer, "%s %s, %d, %05x", jump8long, m_offset, m_brp, m_ppc + m_pc + (INT16) i);
		else
			invalid();
	}

	// memory register
	void inst_mr(const char *instr8, const char *instr16)
	{
		offset();
		if (m_w == 0 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %s", instr8, m_offset, m_reg[m_brp]);
		else if (m_w == 1 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %s", instr16, m_offset, m_reg[m_brp]);
		else
			invalid();
	}

	// pointer memory
	void inst_pm(const char *instr16)
	{
		offset();
		if (m_w == 1 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %s", instr16, m_reg[m_brp], m_offset);
		else
			invalid();
	}

	// memory pointer
	void inst_mp(const char *instr16)
	{
		offset();
		if (m_w == 1 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %s", instr16, m_offset, m_reg[m_brp]);
		else
			invalid();
	}

	// jump memory
	void inst_j16(const char *jump8short, const char *jump16short, const char *jump8long, const char *jump16long)
	{
		offset();
		UINT16 i = fetch_immediate();
		if (m_w == 0 && m_wb == 1)
			sprintf(m_buffer, "%s %s, %05x", jump8short, m_offset, m_ppc + m_pc + (INT8) i);
		else if (m_w == 1 && m_wb == 1)
			sprintf(m_buffer, "%s %s, %05x", jump16short, m_offset, m_ppc + m_pc + (INT8) i);
		else if (m_w == 0 && m_wb == 2)
			sprintf(m_buffer, "%s %s, %05x", jump8long, m_offset, m_ppc + m_pc + (INT16) i);
		else if (m_w == 1 && m_wb == 2)
			sprintf(m_buffer, "%s %s, %05x", jump16long, m_offset, m_ppc + m_pc + (INT16) i);
		else
			invalid();
	}

	// memory
	void inst_m(const char *instr8, const char *instr16)
	{
		offset();
		if (m_w == 0 && m_wb == 0)
			sprintf(m_buffer, "%s %s", instr8, m_offset);
		else if (m_w == 1 && m_wb == 0)
			sprintf(m_buffer, "%s %s", instr16, m_offset);
		else
			invalid();
	}

	// memory bit
	void inst_b(const char *instr)
	{
		offset();
		if (m_w == 0 && m_wb == 0)
			sprintf(m_buffer, "%s %s, %d", instr, m_offset, m_brp);
		else
			invalid();
	}

	// invalid instruction
	void invalid()
	{
		sprintf(m_buffer, "???");
	}

	void disassemble()
	{
		m_pc += 2;

		switch (m_opc)
		{
		case 0x00:
			if (m_mm == 0 && m_w == 0 && m_aa == 0 && m_wb == 0)
			{
				switch (m_brp)
				{
				case 0: sprintf(m_buffer, "nop"); break;
				case 1: invalid(); break;
				case 2: sprintf(m_buffer, "sintr"); break;
				case 3: sprintf(m_buffer, "xfer"); break;
				case 4: sprintf(m_buffer, "wid 8, 8"); break;
				case 5: sprintf(m_buffer, "wid 8, 16"); break;
				case 6: sprintf(m_buffer, "wid 16, 8"); break;
				case 7: sprintf(m_buffer, "wid 16, 16"); break;
				}
			}
			else
				invalid();
			break;

		case 0x02:
			if (m_w == 1 && m_aa == 0 && m_wb == 2)
			{
				UINT16 offs = fetch_immediate();
				UINT16 segm = fetch_immediate();
				sprintf(m_buffer, "lpdi %s, %05x", m_reg[m_brp], ((segm << 4) + offs) & 0xfffff);
			}
			else
				invalid();
			break;

		case 0x08:
			if (m_mm == 0)
			{
				UINT16 i = fetch_immediate();

				if (m_w == 0 && m_aa == 0 && m_wb == 1 && m_brp == TP)
					sprintf(m_buffer, "jmp %05x", m_ppc + m_pc + (INT8) i);
				else if (m_w == 1 && m_aa == 0 && m_wb == 2 && m_brp == TP)
					sprintf(m_buffer, "ljmp %05x", m_ppc + m_pc + (INT16) i);
				else if (m_w == 0 && m_wb == 1)
					sprintf(m_buffer, "addbi %s, %02x", m_reg[m_brp], i);
				else if (m_w == 1 && m_wb == 2)
					sprintf(m_buffer, "addi %s, %04x", m_reg[m_brp], i);
				else
					invalid();
			}
			else
				invalid();
			break;

		case 0x09: inst_ri("orbi", "ori"); break;
		case 0x0a: inst_ri("andbi", "andi"); break;
		case 0x0b: inst_r("not"); break;
		case 0x0c: inst_ri("movbi", "movi"); break;
		case 0x0e: inst_r("inc"); break;
		case 0x0f: inst_r("dec"); break;
		case 0x10: inst_jr("jnz", "ljnz"); break;
		case 0x11: inst_jr("jz", "ljz"); break;

		case 0x12:
			if (m_mm == 0 && m_w == 0 && m_aa == 0 && m_wb == 0 && m_brp == 1)
				sprintf(m_buffer, "hlt");
			else
				invalid();
			break;

		case 0x13: inst_mi("movbi", "movi"); break;
		case 0x20: inst_rm("movb", "mov"); break;
		case 0x21: inst_mr("movb", "mov"); break;
		case 0x22: inst_pm("lpd"); break;
		case 0x23: inst_pm("movp"); break;

		case 0x24:
			if (m_wb == 0)
			{
				offset();

				i8089_instruction *tmp = new i8089_instruction(m_pc, m_oprom + m_pc);
				m_pc += tmp->length();

				if (m_w == 0)
					sprintf(m_buffer, "movb %s, %s", tmp->buffer(), m_offset);
				else
					sprintf(m_buffer, "mov %s, %s", tmp->buffer(), m_offset);

				delete(tmp);
			}
			else
				invalid();
			break;

		case 0x25:
			if (m_w == 0 && m_wb == 3 && m_brp == 0)
			{
				offset();
				UINT16 i = fetch_immediate();
				int displacement = (INT8) fetch_immediate();
				sprintf(m_buffer, "tsl %s, %02x, %05x", m_offset, i, m_ppc + m_pc + displacement);
			}
			else
				invalid();
			break;

		case 0x26: inst_mp("movp"); break;

		case 0x27:
			if (m_w == 1 && m_brp == TP && (m_wb == 1 || m_wb == 2))
			{
				offset();
				UINT16 i = fetch_immediate();

				if (m_wb == 1)
					sprintf(m_buffer, "call %s, %05x", m_offset, m_ppc + m_pc + (INT8) i);
				else if (m_wb == 2)
					sprintf(m_buffer, "lcall %s, %05x", m_offset, m_ppc + m_pc + (INT16) i);

				m_flags |= DASMFLAG_STEP_OVER;
			}
			else
				invalid();
			break;

		case 0x28: inst_rm("addb", "add"); break;
		case 0x29: inst_rm("orb", "or"); break;
		case 0x2a: inst_rm("andb", "and"); break;
		case 0x2b: inst_rm("notb", "not"); break;
		case 0x2c: inst_jm("jmce", "ljmce"); break;
		case 0x2d: inst_jm("jmcne", "ljmcne"); break;
		case 0x2e: inst_jmb("jnbt", "ljnbt"); break;
		case 0x2f: inst_jmb("jbt", "ljbt"); break;
		case 0x30: inst_mi("addbi", "addi"); break;
		case 0x31: inst_mi("orbi", "ori"); break;
		case 0x32: inst_mi("andbi", "andi"); break;

		case 0x33:
			offset();
			sprintf(m_buffer, "%s", m_offset);
			break;

		case 0x34: inst_mr("addb", "add"); break;
		case 0x35: inst_mr("orb", "or"); break;
		case 0x36: inst_mr("andb", "and"); break;
		case 0x37: inst_mr("notb", "not"); break;
		case 0x38: inst_j16("jnzb", "jnz", "ljnzb", "ljnz"); break;
		case 0x39: inst_j16("jzb", "jz", "ljzb", "ljz"); break;
		case 0x3a: inst_m("incb", "inc"); break;
		case 0x3b: inst_m("decb", "dec"); break;
		case 0x3d: inst_b("setb"); break;
		case 0x3e: inst_b("clr"); break;

		default:
			invalid();
		}
	}
};

const char *i8089_instruction::m_reg[] =
{
	"ga", "gb", "gc", "bc", "tp", "ix", "cc", "mc"
};

CPU_DISASSEMBLE( i8089 )
{
	i8089_instruction *i = new i8089_instruction(pc, oprom);
	strcpy(buffer, i->buffer());
	offs_t result = i->length() | i->flags();
	delete(i);
	return result;
}
