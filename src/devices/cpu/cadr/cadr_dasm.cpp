// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    MIT CADR microcode disassembler

The official assembler language is unknown.
An attempt has been made to keep the generated assembler code short and
understandable.

TODO:
- bit 45 ilong

***************************************************************************/

#include "emu.h"
#include "cadr_dasm.h"


namespace {

static constexpr u64 NOP_MASK = u64(0x7fffffffeffff);

static const char *const mult_div_op[0x20] =
{
	"mult-step", "div-step",      "unk", "unk", "unk", "rem-corr", "unk", "unk",
	"unk",       "init-div-step", "unk", "unk", "unk", "unk",      "unk", "unk",
	"mult-step", "div-step",      "unk", "unk", "unk", "rem-corr", "unk", "unk",
	"unk",       "init-div-step", "unk", "unk", "unk", "unk",      "unk", "unk"
};

static const char *const output_bus_control[0x04] =
{
	"ill ", "", ">> ", "<< "
};

static const char *const q_control[0x04] =
{
	"", "<<Q,", ">>Q,", "Q,"
};

static const char *const rp[0x04] =
{
	"branch", "call", "return", "imem-write"
};


void a_source(std::ostream &stream, u64 op)
{
	util::stream_format(stream, "a[%o]", ((op >> 32) & 0x3ff));
}


void m_source(std::ostream &stream, u64 op)
{
	if (BIT(op, 31))
	{
		switch ((op >> 26) & 0x1f)
		{
		case 0x00: stream << "dispatch-constant"; break;
		case 0x01: stream << "SPC-ptr, SPC-data"; break;
		case 0x02: util::stream_format(stream, "PDL-ptr %o", op & 0x3ff); break;
		case 0x03: util::stream_format(stream, "PDL-idx %o", op & 0x3ff); break;
		case 0x04: stream << "PDL-buff--"; break;
		case 0x05: stream << "PDL-buff"; break;
		case 0x06: util::stream_format(stream, "OPC-reg %05o", op & 0x1fff); break;
		case 0x07: stream << "Q"; break;
		case 0x08: stream << "VMA"; break;
		case 0x09: stream << "MAP[MD]"; break;
		case 0x0a: stream << "MD"; break;
		case 0x0b: stream << "LC"; break;
		case 0x0c: stream << "SPC-ptr and data,pop"; break;
		case 0x0d: stream << "reserved"; break;
		case 0x0e: stream << "reserved"; break;
		case 0x0f: stream << "reserved"; break;
		case 0x14: stream << "PDL[Ptr],pop"; break;
		case 0x15: stream << "PDL[Ptr]"; break;
		default:   stream << "illegal"; break;
		}
	}
	else
	{
		util::stream_format(stream, "m[%o]", (op >> 26) & 0x1f);
	}
}


void disassemble_alu_op(std::ostream &stream, u64 op)
{
	if (BIT(op, 8))
	{
		util::stream_format(stream, "%s ", mult_div_op[(op >> 3) & 0x1f]);
		a_source(stream, op);
		stream << " ";
		m_source(stream, op);
		util::stream_format(stream, " C=%d ", BIT(op, 2));
	}
	else
	{
		if (BIT(op, 7))
		{
			if (BIT(op, 2))
			{
				// Arithmetic operations with carry-in
				switch ((op >> 3) & 0x0f)
				{
				case 0x00: // 0
					stream << "0";
					break;
				case 0x01: // M&A
					m_source(stream, op);
					stream << "&";
					a_source(stream, op);
					break;
				case 0x02: // M&~A
					m_source(stream, op);
					stream << "&~";
					a_source(stream, op);
					break;
				case 0x03: // M
					m_source(stream, op);
					break;
				case 0x04: // (M|~A)+1
					stream << "(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")+1";
					break;
				case 0x05: // (M|~A)+(M&A)+1
					stream << "(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")+(";
					m_source(stream, op);
					stream << "&";
					a_source(stream, op);
					stream << ")+1";
					break;
				case 0x06: // M-A
					m_source(stream, op);
					stream << "-";
					a_source(stream, op);
					break;
				case 0x07: // (M|~A)+M+1
					stream << "(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")+";
					m_source(stream, op);
					stream << "+1";
					break;
				case 0x08: // (M|A)+1
					stream << "(";
					m_source(stream, op);
					stream << "|";
					a_source(stream, op);
					stream << ")+1";
					break;
				case 0x09: // M+A+1
					m_source(stream, op);
					stream << "+";
					a_source(stream, op);
					stream << "+1";
					break;
				case 0x0a: // (M|A)+(M&~A)+1
					stream << "(";
					m_source(stream, op);
					stream << "|";
					a_source(stream, op);
					stream << ")+(";
					m_source(stream, op);
					stream << "&~";
					a_source(stream, op);
					stream << ")+1";
					break;
				case 0x0b: // (M|A)+M+1
					stream << "(";
					m_source(stream, op);
					stream << "|";
					a_source(stream, op);
					stream << ")+";
					m_source(stream, op);
					stream << "+1";
					break;
				case 0x0c: // M+1
					m_source(stream, op);
					stream << "+1";
					break;
				case 0x0d: // M+(M&A)+1
					m_source(stream, op);
					stream << "+(";
					m_source(stream, op);
					stream << "&";
					a_source(stream, op);
					stream << ")+1";
					break;
				case 0x0e: // M+(M|~A)+1
					m_source(stream, op);
					stream << "+(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")+1";
					break;
				case 0x0f: // M+M+1
					m_source(stream, op);
					stream << "+";
					m_source(stream, op);
					stream << "+1";
					break;
				}
			}
			else
			{
				// Arithmetic operations without carry-in
				switch ((op >> 3) & 0x0f)
				{
				case 0x00: // -1
					stream << "-1";
					break;
				case 0x01: // (M&A)-1
					stream << "(";
					m_source(stream, op);
					stream << "&";
					a_source(stream, op);
					stream << ")-1";
					break;
				case 0x02: // (M&~A)-1
					stream << "(";
					m_source(stream, op);
					stream << "&~";
					a_source(stream, op);
					stream << ")-1";
					break;
				case 0x03: // M-1
					m_source(stream, op);
					stream << "-1";
					break;
				case 0x04: // M|~A
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					break;
				case 0x05: // (M|~A)+(M&A)
					stream << "(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")+(";
					m_source(stream, op);
					stream << "&";
					a_source(stream, op);
					stream << ")";
					break;
				case 0x06: // M-A-1
					m_source(stream, op);
					stream << "-";
					a_source(stream, op);
					stream << "-1";
					break;
				case 0x07: // (M|~A)+M
					stream << "(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")+";
					m_source(stream, op);
					break;
				case 0x08: // M|A
					m_source(stream, op);
					stream << "|";
					a_source(stream, op);
					break;
				case 0x09: // M+A
					m_source(stream, op);
					stream << "+";
					a_source(stream, op);
					break;
				case 0x0a: // (M|A)+(M&~A)
					stream << "(";
					m_source(stream, op);
					stream << "|";
					a_source(stream, op);
					stream << ")+(";
					m_source(stream, op);
					stream << "&~";
					a_source(stream, op);
					stream << ")";
					break;
				case 0x0b: // (M|A)+M
					stream << "(";
					m_source(stream, op);
					stream << "|";
					a_source(stream, op);
					stream << ")+";
					m_source(stream, op);
					break;
				case 0x0c: // M
					m_source(stream, op);
					break;
				case 0x0d: // M+(M&A)
					m_source(stream, op);
					stream << "+(";
					m_source(stream, op);
					stream << "&";
					a_source(stream, op);
					stream << ")";
					break;
				case 0x0e: // M+(M|~A)
					m_source(stream, op);
					stream << "+(";
					m_source(stream, op);
					stream << "|~";
					a_source(stream, op);
					stream << ")";
					break;
				case 0x0f: // M+M
					m_source(stream, op);
					stream << "+";
					m_source(stream, op);
					break;
				}
			}
		}
		else
		{
			// Boolean operations
			switch ((op >> 3) & 0x0f)
			{
			case 0x00: // SETZ
				stream << "0";
				break;
			case 0x01: // AND
				m_source(stream, op);
				stream << "&";
				a_source(stream, op);
				break;
			case 0x02: // ANDCA
				m_source(stream, op);
				stream << "&~";
				a_source(stream, op);
				break;
			case 0x03: // SETM
				m_source(stream, op);
				break;
			case 0x04: // ANDCM
				stream << "~";
				m_source(stream, op);
				stream << "&";
				a_source(stream, op);
				break;
			case 0x05: // SETA
				a_source(stream, op);
				break;
			case 0x06: // XOR
				m_source(stream, op);
				stream << "^";
				a_source(stream, op);
				break;
			case 0x07: // IOR
				m_source(stream, op);
				stream << "|";
				a_source(stream, op);
				break;
			case 0x08: // ANDCB
				stream << "~";
				m_source(stream, op);
				stream << "&~";
				a_source(stream, op);
				break;
			case 0x09: // EQV
				m_source(stream, op);
				stream << "=";
				a_source(stream, op);
				break;
			case 0x0a: // SETCA
				stream << "~";
				a_source(stream, op);
				break;
			case 0x0b: // ORCA
				m_source(stream, op);
				stream << "|~";
				a_source(stream, op);
				break;
			case 0x0c: // SETCM
				stream << "~";
				m_source(stream, op);
				break;
			case 0x0d: // ORCM
				stream << "~";
				m_source(stream, op);
				stream << "|";
				a_source(stream, op);
				break;
			case 0x0e: // ORCB
				stream << "~";
				m_source(stream, op);
				stream << "|~";
				a_source(stream, op);
				break;
			case 0x0f: // SETO
				stream << "-1";
				break;
			}
		}
	}
}


void disassemble_destination(std::ostream &stream, u64 op)
{
	if (BIT(op, 25))
	{
		util::stream_format(stream, "a[%o] ", (op >> 14) & 0x3ff);
	}
	else
	{
		switch ((op >> 19) & 0x1f)
		{
		case 0x00: stream << ""; break;
		case 0x01: stream << "LC,"; break;
		case 0x02: stream << "IC,"; break;
		case 0x08: stream << "PDL[ptr],"; break;
		case 0x09: stream << "PDL[ptr],push,"; break;
		case 0x0a: stream << "PDL[index],"; break;
		case 0x0b: stream << "PDL-idx,"; break;
		case 0x0c: stream << "PDL-ptr,"; break;
		case 0x0d: stream << "SPC-dat,push,"; break;
		case 0x0e: stream << "OA-reg-lo,"; break;
		case 0x0f: stream << "OA-reg-hi,"; break;
		case 0x10: stream << "VMA,"; break;
		case 0x11: stream << "VMA,start-read,"; break;
		case 0x12: stream << "VMA,start-write,"; break;
		case 0x13: stream << "VMA->write-map,"; break;
		case 0x18: stream << "MD,"; break;
		case 0x19: stream << "MD,start-read,"; break;
		case 0x1a: stream << "MD,start-write,"; break;
		case 0x1b: stream << "MD,write-map,"; break;
		default:   stream << "illegal,"; break;
		}
		util::stream_format(stream, "m[%o] ", (op >> 14) & 0x1f);
	}
}


void disassemble_condition(std::ostream &stream, u64 op)
{
	if (BIT(op, 5))
	{
		switch (op & 0x07)
		{
		case 0x00: stream << "illegal "; break;
		case 0x01: // M < A
			m_source(stream, op);
			stream << "<";
			a_source(stream, op);
			break;
		case 0x02: // M <= A
			m_source(stream, op);
			stream << "<=";
			a_source(stream, op);
			break;
		case 0x03: // M = A
			m_source(stream, op);
			stream << "=";
			a_source(stream, op);
			break;
		case 0x04:
			stream << "pf";
			break;
		case 0x05:
			stream << "pf/int";
			break;
		case 0x06:
			stream << "pf/int/seq";
			break;
		case 0x07:
			stream << "always";
			break;
		}
	}
	else
	{
		m_source(stream, op);
		util::stream_format(stream, "<<%02o", op & 0x1f);
	}
	stream << " ";
}


} // anonymous namespace


u32 cadr_disassembler::opcode_alignment() const
{
	return 1;
}


offs_t cadr_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t cpc = pc;
	offs_t flags = 0;
	u64 op = opcodes.r64(cpc++);

	if (BIT(op, 42))
	{
		stream << "popj, ";
	}

	switch (op & (u64(3) << 43))
	{
	case u64(0) << 43: // ALU
		stream << "alu ";
		if ((op & NOP_MASK) == 0)
		{
			stream << "no-op";
		}
		else
		{
			disassemble_alu_op(stream, op);
			stream << output_bus_control[(op >> 12) & 0x03];
			stream << " -> ";
			stream << q_control[op & 0x03];
			disassemble_destination(stream, op);
		}
		break;
	case u64(1) << 43: // JUMP
		if (((op >> 8) & 0x03) != 0x02)
		{
			util::stream_format(stream, "%s %05o ", rp[(op >> 8) & 0x03], (op >> 12) & 0x3fff);
		}
		else
		{
			util::stream_format(stream, "%s ", rp[(op >> 8) & 0x03]);
		}
		if (BIT(op, 7))
		{
			stream << "!next ";
		}
		if (BIT(op, 6))
		{
			stream << "not ";
		}
		disassemble_condition(stream, op);
		break;
	case u64(2) << 43: // DISPATCH
		stream << "dispatch ";
		if (BIT(op, 25))
		{
			stream << "!N+1 ";
		}
		if (BIT(op, 24))
		{
			stream << "ISH ";
		}
		m_source(stream, op);
		util::stream_format(stream, " disp-const %o, disp-addr %o, map %o, len %o, rot %o", (op >> 32) & 0x2ff,
			(op >> 12) & 0x7ff, (op >> 8) & 0x03, (op >> 5) & 0x07, op & 0x1f
		);
		break;
	case u64(3) << 43: // BYTE
		stream << "byte ";
		a_source(stream, op);
		stream << " ";
		m_source(stream, op);
		stream << " ";
		if (BIT(op, 13))
		{
			if (BIT(op, 12))
			{
				util::stream_format(stream, "dpb pos=%02o, width=%03o ", op & 0x1f, ((op >> 5) & 0x1f) + 1);
			}
			else
			{
				util::stream_format(stream, "sdp pos=%02o, width=%03o ", op & 0x1f, ((op >> 5) & 0x1f) + 1);
			}
		}
		else
		{
			if (BIT(op, 12))
			{
				util::stream_format(stream, "ldb pos=%02o, width=%03o ", op & 0x1f, ((op >> 5) & 0x1f) + 1);
			}
		}
		disassemble_destination(stream, op);
		break;
	}

	return (cpc - pc) | flags | SUPPORTED;
}
